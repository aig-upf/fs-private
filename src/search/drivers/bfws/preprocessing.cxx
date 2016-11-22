
#include <unordered_set>

#include <aptk2/tools/logging.hxx>

#include <search/drivers/bfws/preprocessing.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <ground_state_model.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include <utils/printers/vector.hxx>
#include <search/stats.hxx>

#include <search/algorithms/aptk/generic_search.hxx>
#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>
#include <search/algorithms/aptk/sorted_open_list.hxx>
#include <heuristics/novelty/multivalued_evaluator.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>

#include <search/drivers/bfws/ctmp_features.hxx>
#include "enhanced_bfws.hxx"
#include <search/drivers/setups.hxx>

using namespace fs0;
namespace fs = fs0::language::fstrips;

//! a helper
template <typename StateT>
void derive_all_atoms(const fs::Formula* formula, const StateT& state, std::set<fs0::Atom>& relevant) {
	const fs::Conjunction* precs = dynamic_cast<const fs::Conjunction*>(formula);
	if (!precs) throw std::runtime_error("This driver can only be applied to a conjunctive-action-precondition problems");
	
	fs0::Binding _;
	for (const fs::StateVariable* sv:fs0::Utils::filter_by_type<const fs::StateVariable*>(formula->all_terms())) {
		relevant.insert(fs0::Atom(sv->getValue(), sv->interpret(state, _)));
	}
}

template <typename StateT, typename ActionType>
class IWPreprocessingNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IWPreprocessingNode<StateT, ActionT>>;
	StateT state;
	
	FeatureValuation feature_valuation;
	
	unsigned type; // The type of novelty
	
	typename ActionT::IdType action;

	PT parent;
	
// 	std::set<unsigned> _violated_state_constraint_atoms;
	
	
	IWPreprocessingNode() = delete;
	~IWPreprocessingNode() = default;
	IWPreprocessingNode(const IWPreprocessingNode&) = delete;
	IWPreprocessingNode(IWPreprocessingNode&&) = delete;
	IWPreprocessingNode& operator=(const IWPreprocessingNode&) = delete;
	IWPreprocessingNode& operator=(IWPreprocessingNode&&) = delete;
	
	//! Constructor with full copying of the state (expensive)
	IWPreprocessingNode( const StateT& s )
		: state( s ), action( ActionT::invalid_action_id ), parent( nullptr )
// 		, _violated_state_constraint_atoms()
	{}

	//! Constructor with move of the state (cheaper)
	IWPreprocessingNode( StateT&& _state, typename ActionT::IdType _action, PT& _parent ) :
		state(std::move(_state)),
		action(_action),
		parent(_parent)
// 		_violated_state_constraint_atoms(_parent->_violated_state_constraint_atoms)
	{}

	bool has_parent() const { return parent != nullptr; }

		//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWPreprocessingNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		os << "{@ = " << this << ", s = " << state << ", features= " << fs0::print::container(feature_valuation) << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const IWPreprocessingNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
	
	
// 	void state_constraint_violation(unsigned sc_atom_idx) {
// 		_violated_state_constraint_atoms.insert(sc_atom_idx);
// 	}
	
// 	std::size_t num_violations() {
// 		return _violated_state_constraint_atoms.size();
// 	}
};


namespace lapkt {

//! Partial specialization of the GenericSearch algorithm:
//! A breadth-first search is a generic search with a FIFO open list and 
//! a standard unsorted closed list. Type of node and state model are still generic.
template <typename NodeT,
          typename StateModel,
          typename OpenListT = aptk::StlUnsortedFIFO<NodeT>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class AllSolutionsBreadthFirstSearch : public GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>
{
public:
	using ActionT = typename StateModel::ActionType;
	using Base = GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	using StateT = typename StateModel::StateType;
	using PlanT = typename Base::PlanT;
	using NodePT = typename Base::NodePtr;
	
	using NodeOpenEvent = typename Base::NodeOpenEvent;
	using NodeExpansionEvent = typename Base::NodeExpansionEvent;
	using NodeCreationEvent = typename Base::NodeCreationEvent;

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	AllSolutionsBreadthFirstSearch(const StateModel& model, OpenListT&& open, const std::vector<const fs::AtomicFormula*>& goal, const std::vector<const fs::AtomicFormula*>& state_constraints, bool all_solutions) :
		Base(model, std::move(open), ClosedListT()),
		_all_solutions(all_solutions),
		_goal_atoms(goal), _sc_atoms(state_constraints),
		_optimal_paths(_goal_atoms.size(), nullptr)
	{}
	virtual ~AllSolutionsBreadthFirstSearch() = default;
	
	// Disallow copy, but allow move
	AllSolutionsBreadthFirstSearch(const AllSolutionsBreadthFirstSearch&) = delete;
	AllSolutionsBreadthFirstSearch(AllSolutionsBreadthFirstSearch&&) = default;
	AllSolutionsBreadthFirstSearch& operator=(const AllSolutionsBreadthFirstSearch&) = delete;
	AllSolutionsBreadthFirstSearch& operator=(AllSolutionsBreadthFirstSearch&&) = default;
	
	bool search(const StateT& s, PlanT& solution) override {
		NodePT n = std::make_shared<NodeT>(s);
		this->notify(NodeCreationEvent(*n));
		this->_open.insert(n);
		
		while ( !this->_open.is_empty() ) {
			NodePT current = this->_open.get_next( );
			this->notify(NodeOpenEvent(*current));
			
			bool goals_reached = process_node(current);
			if (!_all_solutions && goals_reached) {
				break;
			}

			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			this->_closed.put(current);
			
			this->notify(NodeExpansionEvent(*current));
			
			for ( const auto& a : this->_model.applicable_actions( current->state ) ) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current );
				
// 				progress_node(successor);
				
				if (this->_closed.check(successor)) continue; // The node has already been closed
				
				this->notify(NodeCreationEvent(*successor));
				if (!this->_open.insert( successor )) {
					LPT_DEBUG("search", std::setw(7) << "PRUNED: " << *successor);
				}
			}
		}
		
		return final_process();
	}
	
protected:
	//! Whether we want to compute IW up until the end or only until paths to all subgoals
	//! are found _for the first time_
	bool _all_solutions;
	
	//! An index of the nodes with are in the open list at any moment, for faster access
// 	using node_unordered_set = std::unordered_set<NodePT, node_hash<NodePT>, node_equal_to<NodePT>>;
// 	node_unordered_set _solutions;
	
	const std::vector<const fs::AtomicFormula*>& _goal_atoms;
	const std::vector<const fs::AtomicFormula*>& _sc_atoms;
	
	//! _optimal_paths[i] contains the node with a minimum number of state constraint violations
	//! that reaches the goal atom 'i'.
	std::vector<NodePT> _optimal_paths;
	
// 	void progress_node(NodePT& node) {
// 		const StateT& state = node->state;
// 		for (unsigned sc_atom_idx = 0; sc_atom_idx < _sc_atoms.size(); ++sc_atom_idx) {
// 			const fs::AtomicFormula* atom = _sc_atoms[sc_atom_idx];
// 			if (!atom->interpret(state)) {
// 				// The problem state violates the state constraint atom 'atom'
// 				node->state_constraint_violation(sc_atom_idx);
// 			}
// 		}
// 	}
	
	bool process_node(const NodePT& node) {
		const StateT& state = node->state;
		unsigned num_satisfied = 0;
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			NodePT& optimal = _optimal_paths[goal_atom_idx];
			
			if (optimal) continue; // We are glad with the first path we found
			
			if (atom->interpret(state)) { // The state satisfies goal atom with index 'i'
				optimal = node;
				++num_satisfied;
// 				if (!optimal) {
// 					LPT_INFO("cout", "PREPROCESSING: Goal atom '" << *atom << "' reached for the first time");
// 				}
// 				if (!optimal || optimal->num_violations() > node->num_violations()) {
// 					optimal = node;
// 				}
			}
			
// 			if (optimal) {
// 				++num_satisfied;
// 			}
		}
		
		return num_satisfied == _goal_atoms.size();
	}
	
	//! Returns true iff all goal atoms have been reached
	bool final_process() {
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			const NodePT& optimal = _optimal_paths[goal_atom_idx];
			if (!optimal) { // 
				LPT_INFO("cout", "PREPROCESSING: Goal atom '" << *atom << "' is not reachable!");
				return false;
			}
		}
		return true;
	}

public:
	std::set<fs0::Atom> retrieve_relevant_atom_sets() {
		
		std::set<fs0::Atom> relevant;
		
		const fs0::Problem& problem = fs0::Problem::getInstance();
		const auto& actions = problem.getGroundActions();
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			// const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			NodePT node = _optimal_paths[goal_atom_idx];
			assert(node);
			
			while (node->has_parent()) {
				const ActionT& action = *actions.at(node->action);
				node = node->parent;
				derive_all_atoms(action.getPrecondition(), node->state, relevant);
			}
		}
		
		return relevant;
	}
	
	
	//! One offending set per each goal atom
	std::vector<OffendingSet> compute_offending_configurations() {
		std::vector<OffendingSet> offending(_goal_atoms.size());
		
		const ProblemInfo& info = ProblemInfo::getInstance();
		const Problem& problem = Problem::getInstance();
		const ExternalI& external = info.get_external();
		
		const auto& ground_actions = problem.getGroundActions();
		assert(ground_actions.size());
		
		VariableIdx v_confb = info.getVariableId("confb(rob)");
		VariableIdx v_traja = info.getVariableId("traj(rob)");
		VariableIdx v_holding = info.getVariableId("holding()");
		ObjectIdx no_object_id = info.getObjectId("no_object");
		_unused(no_object_id);
		
		// First, precompute which is the goal configuration of every object, if any
		std::unordered_map<ObjectIdx, ObjectIdx> object_goal;
		for (ObjectIdx obj_id:info.getTypeObjects("object_id")) {
			// If the object has a particular goal configuration, insert it.
			ObjectIdx goal_config = fs0::drivers::derive_goal_config(obj_id, _goal_atoms);
			if (goal_config != -1) {
				object_goal.insert(std::make_pair(obj_id, goal_config));
			}
		}
		
		
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			// const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			NodePT node = _optimal_paths[goal_atom_idx];
			assert(node);
			
			while (node->has_parent()) {
				const StateT& state = node->state;
// 				const StateT& parent_state = node->parent->state;
				ActionIdx action_id = node->action;
				const GroundAction& action = *(ground_actions.at(action_id));
				
				// COMPUTE ALL OBJECT CONFIGURATIONS THAT (AT ANY TIME) CAN OVERLAP WITH THE POSITION OF THE ROBOT IN THIS STATE
				if (action.getName() == "transition_arm") {
					
					ObjectIdx o_confb = state.getValue(v_confb);
					ObjectIdx o_traj_arm = state.getValue(v_traja);
					ObjectIdx o_held = state.getValue(v_holding);
					auto v_off = external.get_offending_configurations(o_confb, o_traj_arm, o_held);
					offending[goal_atom_idx].insert(v_off.begin(), v_off.end());
				}
				
				/*
				// ADDITIONALLY, IF THE ACTION USED TO REACH THIS STATE IS A 'PLACE' ACTION THAT PUTS THE PLACED OBJECT
				// INTO ITS GOAL CONFIGURATION, THEN THIS GOAL CONFIGURATION IS MARKED AS A POTENTIALLY OFFENDING CONFIGURATION AS WELL
				
				if (action.getActionData().getName() == "place-object") {
					ObjectIdx held = parent_state.getValue(v_holding);
					assert(held!=no_object_id);
					
					std::string obj_name = info.deduceObjectName(held, info.getTypeId("object_id"));
					VariableIdx obj_conf = info.getVariableId("confo(" + obj_name  +  ")");
					ObjectIdx current_config = state.getValue(obj_conf);
	
					auto it = object_goal.find(held);
					if (it != object_goal.end() && current_config == it->second) {
						offending[goal_atom_idx].insert(current_config);
					}
				}
				*/
				
				node = node->parent;
			}
		}
		
		return offending;
	}
}; 

} // Namespaces

namespace fs0 { namespace drivers {


std::vector<OffendingSet>
EBFWSPreprocessor::preprocess(const Problem& problem, const Config& config, BasicApplicabilityAnalyzer* analyzer) {
	
	const fs::Conjunction* original_goal = dynamic_cast<const fs::Conjunction*>(problem.getGoalConditions());
	if (!original_goal) {
		throw std::runtime_error("This driver can only be applied to a conjunctive-goal problems");
	}
	const std::vector<const fs::AtomicFormula*>& goal_conjuncts = original_goal->getConjuncts();
	
	std::vector<const fs::AtomicFormula*> sc_conjuncts;
	if (problem.getStateConstraints()->is_tautology()) {
		
	} else {
		const fs::Conjunction* original_sc = dynamic_cast<const fs::Conjunction*>(problem.getStateConstraints());
		if (!original_sc) {
			throw std::runtime_error("This driver can only be applied to a conjunctive-state-constraints problems");
		}
		sc_conjuncts = original_sc->getConjuncts();
	}
	
	SearchStats preprocessing_stats;
	std::set<unsigned> relevant_atoms;
	
	NoveltyFeaturesConfiguration feature_configuration(config);
	unsigned k = 2; // Use IW(2)
	
	
	Problem simplified(problem);
	simplified.set_state_constraints(new fs::Tautology);
	GroundStateModel model(simplified, analyzer);
	
	using PreprocessingNodeT = IWPreprocessingNode<State, GroundAction>;
	using AcceptorT = CTMPOpenListAcceptor<GroundStateModel, PreprocessingNodeT>;
	using AcceptorPT = std::shared_ptr<AcceptorT>;
	using OpenListT = aptk::StlUnsortedFIFO<PreprocessingNodeT, AcceptorT>;
	using BaseAlgoT = lapkt::AllSolutionsBreadthFirstSearch<PreprocessingNodeT, GroundStateModel, OpenListT>;
	
	bool use_all_solutions = config.getOption<std::string>("ebfws.iw") == "all";
	
	AcceptorPT evaluator = std::shared_ptr<AcceptorT>(new AcceptorT(model, k, feature_configuration));
// 	AcceptorPT evaluator = std::make_shared<AcceptorT>(model, k, feature_configuration);
	
	
	BaseAlgoT iw_algorithm(model, OpenListT(evaluator), goal_conjuncts, sc_conjuncts, use_all_solutions);
// 		lapkt::events::subscribe(*_algorithm, _handlers);
	
	
	SearchStats stats;
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> handlers;
	EventUtils::setup_stats_observer<PreprocessingNodeT>(stats, handlers);
	lapkt::events::subscribe(iw_algorithm, handlers);
	
	
	std::vector<ActionIdx> plan;
	LPT_INFO("cout", "PREPROCESSING: Starting IW(2) search...");
	bool solved = iw_algorithm.search(simplified.getInitialState(), plan);
	
	LPT_INFO("cout", "PREPROCESSING: Finished");
	LPT_INFO("cout", "PREPROCESSING: Node expansions: " << stats.expanded());
	LPT_INFO("cout", "PREPROCESSING: Node generations: " << stats.generated());
	
	if (!solved) throw std::runtime_error("PREPROCESSING - IW(2) preprocessing did not yield a solution for all subgoals");
	
	LPT_INFO("cout", "PREPROCESSING: Computing offending configuration sets...");
	std::vector<OffendingSet> offending = iw_algorithm.compute_offending_configurations();
	
	/*
	LPT_INFO("cout", "PREPROCESSING: Retrieving relevant atoms...");
	auto relevant = iw_algorithm.retrieve_relevant_atom_sets();
	std::cout << "PREPROCESSING - RELEVANT ATOMS ARE: " << std::endl;
	for (const auto& atom:relevant) std::cout << atom << ", ";
	std::cout << std::endl;
	*/
	
	return offending;
}

	
	
	
} } // namespaces
