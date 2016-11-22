
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
		_info(ProblemInfo::getInstance()),
		_tuple_idx(model.getTask().get_tuple_index()),
		_init(model.init()),
		_all_solutions(all_solutions),
		_goal_atoms(goal), _sc_atoms(state_constraints),
		_optimal_paths(_goal_atoms.size(), nullptr),
		_tuple_to_node(_tuple_idx.size(), nullptr),
		_obj_to_holding_tuple_idx(_info.getNumObjects(), -1)
	{
		VariableIdx holding_v = _info.getVariableId("holding()");
		for (ObjectIdx obj:_info.getTypeObjects("object_id")) {
			TupleIdx t = _tuple_idx.to_index(holding_v, obj); // i.e. the tuple index of the atom holding()=o
			_obj_to_holding_tuple_idx.at(obj) = t;
		}
	
	
	}
	
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
	const ProblemInfo& _info;
	const TupleIndex& _tuple_idx;
	const StateT _init;
	
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
	
	//! The node first achieving each problem tuple
	std::vector<NodePT> _tuple_to_node;
	
	//! Object_id o to the index of the tuple "holding()=o"
	std::vector<TupleIdx> _obj_to_holding_tuple_idx;
	
	
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
		
		for (unsigned i = 0; i < state.numAtoms(); ++i) {
			TupleIdx idx = _tuple_idx.to_index(i, state.getValue(i));
			if (_tuple_to_node[idx] == nullptr) {
				_tuple_to_node[idx] = node;
			}
		}
		
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
		
		return false; // RUN IW(2) ALWAYS TIL THE END
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
	
	//! One offending set per each goal atom
	std::vector<OffendingSet> compute_offending_configurations() {
		std::vector<OffendingSet> offending_0 = compute_goal_offending_configurations();
		
		const std::vector<ObjectIdx> all_objects = _info.getTypeObjects("object_id");
		
		// Now augment this set as follows:
		// Taking each offending configuration, and check whether some object o is in that config in the initial state. If not, continue with the next configuration.
		// If yes, flag as "offending" all the configurations that themselves offend the precomputed trajectory to reach the atom "holding(o)".
		// Continue recursively.
		for (unsigned i = 0; i < offending_0.size(); ++i) {
			OffendingSet& offending = offending_0[i];
			
			unsigned offending0_size = offending.size();
			unsigned cur_size = 0;
		
			std::vector<bool> processed(all_objects.size(), false); // This will tell us for each object whether the tuple holding(o) has already been processed.
			
			do {
				cur_size = offending.size();
				
				for (unsigned j = 0; j < all_objects.size(); ++j) {
					ObjectIdx obj = all_objects[j];
					if (processed[j]) continue; // No need to process twice the same holding(o) subgoal!
					
					VariableIdx confo_var = _info.getVariableId("confo(" + _info.deduceObjectName(obj, "object_id") + ")"); // TODO This should be precomputed
					ObjectIdx confo = _init.getValue(confo_var);
					if (offending.find(confo) == offending.end()) continue;
					
					// Otherwise, object o is initially in an offending configuration
					
					TupleIdx holding_o = _obj_to_holding_tuple_idx.at(obj); // the tuple "holding(o)"
					NodePT& node = _tuple_to_node.at(holding_o);
					flag_offending_configurations(node, offending);
					processed[j] = true;
				}
				
			} while(offending.size() > cur_size);
			
			
			LPT_INFO("cout", "A total of " << offending.size() << " (" << offending0_size << " + " << offending.size() - offending0_size << ") real object configurations found to be offending to goal atom " << *_goal_atoms[i]);
		}
		
		// print_offending_graph(offending_0);
		
		return offending_0;
	}
	
	void print_offending_graph(const std::vector<OffendingSet>& all_offending) {
		
		VariableIdx holding_v = _info.getVariableId("holding()");
		for (ObjectIdx obj:_info.getTypeObjects("object_id")) {
			std::string o1_name = _info.deduceObjectName(obj, "object_id");
			TupleIdx t = _tuple_idx.to_index(holding_v, obj); // i.e. the tuple index of the atom holding()=o
			
			OffendingSet offending;
			NodePT& node = _tuple_to_node.at(t);
			flag_offending_configurations(node, offending);
			
			print_offending_objects("holding(" + o1_name + ")", offending);
		}
		
		LPT_INFO("deadlocks", "");LPT_INFO("deadlocks", "");LPT_INFO("deadlocks", "");LPT_INFO("deadlocks", "");
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			LPT_INFO("deadlocks", "Goal Atom " << *atom << " offended by: ");
			print_offending_objects("", all_offending.at(goal_atom_idx));
		}
		
	}
	
	void print_offending_objects(const std::string& to, const OffendingSet& offending) {
		
		for (ObjectIdx obj2:_info.getTypeObjects("object_id")) {
			std::string o2_name = _info.deduceObjectName(obj2, "object_id");
			VariableIdx confo_var = _info.getVariableId("confo(" + o2_name + ")");
			ObjectIdx confo = _init.getValue(confo_var);
			if (offending.find(confo) != offending.end()) {
				LPT_INFO("deadlocks", to << " - offended by " << o2_name);
			}
		}
	}
	
//! One offending set per each goal atom
	std::vector<OffendingSet> compute_goal_offending_configurations() {
		std::vector<OffendingSet> offending(_goal_atoms.size());

		// First, precompute which is the goal configuration of every object, if any
		std::unordered_map<ObjectIdx, ObjectIdx> object_goal;
		for (ObjectIdx obj_id:_info.getTypeObjects("object_id")) {
			// If the object has a particular goal configuration, insert it.
			ObjectIdx goal_config = fs0::drivers::derive_goal_config(obj_id, _goal_atoms);
			if (goal_config != -1) {
				object_goal.insert(std::make_pair(obj_id, goal_config));
			}
		}
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			NodePT& node = _optimal_paths[goal_atom_idx];
			assert(node);
			flag_offending_configurations(node, offending[goal_atom_idx]);
		}
		
		return offending;
	}
	
	// Flag as offending all real object configurations that offend the arm trajectories of the
	// path from the root to the given node
	void flag_offending_configurations(NodePT& node, OffendingSet& offending) {
		const ExternalI& external = _info.get_external();
		const auto& ground_actions = this->_model.getTask().getGroundActions();
		assert(ground_actions.size());
		VariableIdx v_confb = _info.getVariableId("confb(rob)");
		VariableIdx v_traja = _info.getVariableId("traj(rob)");
		VariableIdx v_holding = _info.getVariableId("holding()");
		
		while (node->has_parent()) {
			const StateT& state = node->state;
			// const StateT& parent_state = node->parent->state;
			ActionIdx action_id = node->action;
			const GroundAction& action = *(ground_actions.at(action_id));
			
			// COMPUTE ALL OBJECT CONFIGURATIONS THAT (AT ANY TIME) CAN OVERLAP WITH THE POSITION OF THE ROBOT IN THIS STATE
			if (action.getName() == "transition_arm") {
				ObjectIdx o_confb = state.getValue(v_confb);
				ObjectIdx o_traj_arm = state.getValue(v_traja);
				ObjectIdx o_held = state.getValue(v_holding);
				auto v_off = external.get_offending_configurations(o_confb, o_traj_arm, o_held);
				offending.insert(v_off.begin(), v_off.end());
			}
			node = node->parent;
		}
	}
	
	/*
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
	}*/
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
