
#include <unordered_set>

#include <search/drivers/bfws/enhanced_bfws.hxx>
#include "bfws_f5.hxx"
#include <search/drivers/iterated_width.hxx>
#include <search/utils.hxx>
#include <utils/utils.hxx>
#include <utils/external.hxx>

using namespace fs0;

extern std::unique_ptr<ExternalI> external;


//! a helper
template <typename StateT>
void derive_all_atoms(const fs::Formula* formula, const StateT& state, std::set<fs0::Atom>& relevant) {

	const fs::Conjunction* precs = dynamic_cast<const fs::Conjunction*>(formula);
	if (!precs) throw std::runtime_error("This driver can only be applied to a conjunctive-action-precondition problems");
	
	fs0::Binding _;
	for (const fs::StateVariable* sv:fs0::Utils::filter_by_type<const fs::StateVariable*>(formula->all_terms())) {
		relevant.insert(fs0::Atom(sv->getValue(), sv->interpret(state, _)));
	}
		
// 	const std::vector<const fs::AtomicFormula*>& conjuncts = precs->getConjuncts();
// 	for (auto conjunct:conjuncts) {
// 		std::cout << "CONJUNCT: " << *conjunct << std::endl;
// 
// 		auto atomic = dynamic_cast<const fs::EQAtomicFormula*>(conjunct);
// 		if (!atomic) throw std::runtime_error("This driver can only be applied to problems with simple equality atoms in action preconditions");
// 		
// 		auto rhs = dynamic_cast<const fs::Constant*>(atomic->rhs());
// 		if (!rhs) {
// 			throw std::runtime_error("This driver can only be applied to problems with simple equality atoms in action preconditions");
// 		}
		
// 		auto lhs = dynamic_cast<const fs::StateVariable*>(atomic->lhs());
// 	}
}


template <typename StateT, typename ActionType>
class IWPreprocessingNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IWPreprocessingNode<StateT, ActionT>>;
	StateT state;
	
	typename ActionT::IdType action;

	PT parent;
	
	std::set<unsigned> _violated_state_constraint_atoms;
	
	
	IWPreprocessingNode() = delete;
	~IWPreprocessingNode() = default;
	IWPreprocessingNode(const IWPreprocessingNode&) = delete;
	IWPreprocessingNode(IWPreprocessingNode&&) = delete;
	IWPreprocessingNode& operator=(const IWPreprocessingNode&) = delete;
	IWPreprocessingNode& operator=(IWPreprocessingNode&&) = delete;
	
	//! Constructor with full copying of the state (expensive)
	IWPreprocessingNode( const StateT& s )
		: state( s ), action( ActionT::invalid_action_id ), parent( nullptr ), _violated_state_constraint_atoms()
	{}

	//! Constructor with move of the state (cheaper)
	IWPreprocessingNode( StateT&& _state, typename ActionT::IdType _action, PT& _parent ) :
		state(std::move(_state)),
		action(_action),
		parent(_parent),
		_violated_state_constraint_atoms(_parent->_violated_state_constraint_atoms)
	{}

	bool has_parent() const { return parent != nullptr; }

		//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWPreprocessingNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		os << "{@ = " << this << ", s = " << state << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const IWPreprocessingNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
	
	
	void state_constraint_violation(unsigned sc_atom_idx) {
		_violated_state_constraint_atoms.insert(sc_atom_idx);
	}
	
	std::size_t num_violations() {
		return _violated_state_constraint_atoms.size();
	}
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
	using BaseClass = GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	using StateT = typename StateModel::StateType;
	using PlanT = typename BaseClass::PlanT;
	using NodePT = typename BaseClass::NodePtr;

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	AllSolutionsBreadthFirstSearch(const StateModel& model, OpenListT&& open, const std::vector<const fs::AtomicFormula*>& goal, const std::vector<const fs::AtomicFormula*>& state_constraints) :
		BaseClass(model, std::move(open), ClosedListT()), _goal_atoms(goal), _sc_atoms(state_constraints),
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
		this->_open.insert(n);
		
		while ( !this->_open.is_empty() ) {
			NodePT current = this->_open.get_next( );
			
			process_node(current);

			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			this->_closed.put(current);
			
			for ( const auto& a : this->_model.applicable_actions( current->state ) ) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current );
				
				progress_node(successor);
				
				if (this->_closed.check(successor)) continue; // The node has already been closed
				if (this->_open.updatable(successor)) continue; // The node is currently on the open list, we update some of its attributes but there's no need to reinsert it.
				
				this->_open.insert( successor );
			}
		}
		
		return final_process();
	}
	
protected:
	//! An index of the nodes with are in the open list at any moment, for faster access
// 	using node_unordered_set = std::unordered_set<NodePT, node_hash<NodePT>, node_equal_to<NodePT>>;
// 	node_unordered_set _solutions;
	
	const std::vector<const fs::AtomicFormula*>& _goal_atoms;
	const std::vector<const fs::AtomicFormula*>& _sc_atoms;
	
	//! _optimal_paths[i] contains the node with a minimum number of state constraint violations
	//! that reaches the goal atom 'i'.
	std::vector<NodePT> _optimal_paths;
	
	void progress_node(NodePT& node) {
		const StateT& state = node->state;
		for (unsigned sc_atom_idx = 0; sc_atom_idx < _sc_atoms.size(); ++sc_atom_idx) {
			const fs::AtomicFormula* atom = _sc_atoms[sc_atom_idx];
			if (!atom->interpret(state)) {
				// The problem state violates the state constraint atom 'atom'
				node->state_constraint_violation(sc_atom_idx);
			}
		}
	}
	
	void process_node(const NodePT& node) {
		const StateT& state = node->state;
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			if (atom->interpret(state)) {
				//LPT_INFO("cout", "PREPROCESSING: Goal atom '" << *atom << "' reached");
				// The state satisfies goal atom with index 'i'
				const NodePT& optimal = _optimal_paths[goal_atom_idx];
				if (!optimal || optimal->num_violations() > node->num_violations()) {
					_optimal_paths[goal_atom_idx] = node;
				}
			}
		}
	}
	
	//! Returns true iff all goal atoms have been reached
	bool final_process() {
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			const NodePT& optimal = _optimal_paths[goal_atom_idx];
			if (!optimal) { // 
				LPT_INFO("cout", "PREPROCESSING: Goal atom '" << *atom << "' not reachable");
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
		
// 		const fs0::Problem& problem = fs0::Problem::getInstance();
		const ProblemInfo& info = ProblemInfo::getInstance();
		const ExternalI& external = info.get_external();
		
		
		VariableIdx v_confb = info.getVariableId("confb(rob)");
		VariableIdx v_confa = info.getVariableId("confa(rob)");
		
		
		
		for (unsigned goal_atom_idx = 0; goal_atom_idx < _goal_atoms.size(); ++goal_atom_idx) {
			// const fs::AtomicFormula* atom = _goal_atoms[goal_atom_idx];
			NodePT node = _optimal_paths[goal_atom_idx];
			assert(node);
			
			while (node->has_parent()) {
				const StateT& state = node->state;
				
				// COMPUTE ALL OBJECT CONFIGURATIONS THAT OVERLAP WITH THE POSITION OF THE ROBOT IN THIS STATE
				ObjectIdx o_confb = state.getValue(v_confb);
				ObjectIdx o_confa = state.getValue(v_confa);
				auto v_off = external.get_offending_configurations(o_confb, o_confa);
				offending[goal_atom_idx].insert(v_off.begin(), v_off.end());
				
				node = node->parent;
			}
		}
		
		return offending;
	}
	
	
}; 

} // Namespaces

namespace fs0 { namespace drivers {


	
class BFWSF6Node {
public:
	using ptr_t = std::shared_ptr<BFWSF6Node>;
	
	State state;
	GroundAction::IdType action;
	
	ptr_t parent;

	//! Accummulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;
	
	//! Number of unachieved goals (#g)
	unsigned unachieved;
	
	//! The number of objects in an offending configuration
	unsigned _num_offending;
	
	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
	enum class REACHED_STATUS : unsigned char {IRRELEVANT_ATOM, UNREACHED, REACHED};
	using Atomset = std::vector<REACHED_STATUS>;
	
	Atomset _reached_idx;
	unsigned _num_relaxed_achieved;
	
	//!
	long hff;
	
public:
	BFWSF6Node() = delete;
	~BFWSF6Node() = default;
	
	BFWSF6Node(const BFWSF6Node& other) = delete;
	BFWSF6Node(BFWSF6Node&& other) = delete;
	BFWSF6Node& operator=(const BFWSF6Node& rhs) = delete;
	BFWSF6Node& operator=(BFWSF6Node&& rhs) = delete;
	
	//! Constructor with full copying of the state (expensive)
	BFWSF6Node(const State& s) : BFWSF6Node(State(s), GroundAction::invalid_action_id, nullptr) {}

	//! Constructor with move of the state (cheaper)
	BFWSF6Node(State&& _state, GroundAction::IdType action_, ptr_t parent_) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0),
		novelty(std::numeric_limits<unsigned>::max()),
		unachieved(std::numeric_limits<unsigned>::max()),
		_num_offending(std::numeric_limits<unsigned>::max()),
		_num_relaxed_achieved(0),
		hff(std::numeric_limits<long>::max())
	{}

	bool has_parent() const { return parent != nullptr; }
	
	//! Required for the interface of some algorithms that might prioritise helpful actions.
	bool is_helpful() const { return false; }


	bool operator==( const BFWSF6Node& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const BFWSF6Node& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		std::string hval = hff == std::numeric_limits<long>::max() ? "?" : std::to_string(hff);
		return os << "{@ = " << this << ", s = " << state << ", g = " << g << ", w = " << novelty << ", #g=" << unachieved << ", #off=" << _num_offending << ", #r=" << _num_relaxed_achieved << ", h = " << hval << ", parent = " << parent << "}";
	}
	
	void update_reached_counters(const State& state) {
		const ProblemInfo& info = ProblemInfo::getInstance();
		const TupleIndex& tuple_idx = Problem::getInstance().get_tuple_index();
		
		// We copy the map of reached values from the parent node and will update it below
		_reached_idx = parent->_reached_idx;
		_num_relaxed_achieved = parent->_num_relaxed_achieved;
		
		for (VariableIdx var = 0; var < state.numAtoms(); ++var) {
			
			// We perform the check only for functional variables or non-false predicative variables,
			// as 0-valued predicative variables won't be part of the set of atoms anyway.
			// TODO This should be revised if we change the definition of the set of relevant atoms
			// TODO and it includes possible 0-valued predicative atoms
			if (!info.isPredicativeVariable(var) || state.getValue(var) != 0) {
				TupleIdx index = tuple_idx.to_index(Atom(var, state.getValue(var)));
				auto& element = _reached_idx[index];
				if (element == REACHED_STATUS::UNREACHED) {
					element = REACHED_STATUS::REACHED;
					++_num_relaxed_achieved;
				}
			}
		}
	}
	
	template <typename Heuristic>
	void evaluate_with( Heuristic& ensemble ) {
		unachieved = ensemble.get_unachieved(this->state);
		_num_offending = ensemble.compute_offending(this->state);
		
		if (!has_parent() || unachieved < parent->unachieved) {
			// TODO Is the initialization of _num_relaxed_achieved correct?
// 			hff = ensemble.compute_heuristic(state, _reached_idx, _num_relaxed_achieved);
		} else {
// 			update_reached_counters(this->state);
		}
		
		novelty = ensemble.novelty(state, unachieved, _num_offending);
		if (novelty > ensemble.max_novelty()) {
			novelty = std::numeric_limits<unsigned>::max();
		}
	}
	
	void inherit_heuristic_estimate() {
		if (parent) {
			novelty = parent->novelty;
			hff = parent->hff;
			unachieved = parent->unachieved;
			_reached_idx = parent->_reached_idx;
			_num_relaxed_achieved = parent->_num_relaxed_achieved;
			_num_offending = parent->_num_offending;
		}
	}
	
	//! What to do when an 'other' node is found during the search while 'this' node is already in
	//! the open list
	void update_in_open_list(ptr_t other) {
		if (other->g < this->g) {
			this->g = other->g;
			this->action = other->action;
			this->parent = other->parent;
			this->novelty = other->novelty;
			this->hff = other->hff;
			unachieved = other->unachieved;
			_num_relaxed_achieved = other->_num_relaxed_achieved;
			_reached_idx = other->_reached_idx;
			_num_offending = other->_num_offending;
		}
	}

	bool dead_end() const { return hff == -1; }
};



struct F6NodeComparer {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty < n2->novelty) return true;
		if (n1->novelty > n2->novelty) return false;
		
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;

		return n1->g > n2->g;
	}
};



template <typename StateModelT, typename BaseHeuristicT>
class BFWSF56Heuristic : public BFWSF5HeuristicEnsemble<StateModelT, BaseHeuristicT> {
public:
	using BaseT = BFWSF5HeuristicEnsemble<StateModelT, BaseHeuristicT>;
	
	BFWSF56Heuristic(const StateModelT& model, unsigned max_novelty, const NoveltyFeaturesConfiguration& feature_configuration, std::unique_ptr<BaseHeuristicT>&& heuristic, std::vector<OffendingSet>&& offending) :
		BaseT(model, max_novelty, feature_configuration, std::move(heuristic)),
		_offending(std::move(offending))
	{
		const ProblemInfo& info = ProblemInfo::getInstance();
		TypeIdx obj_t = info.getTypeId("object_id");
		for (ObjectIdx obj_id:info.getTypeObjects(obj_t)) {
			std::string obj_name = info.deduceObjectName(obj_id, obj_t);
			VariableIdx confo_var = info.getVariableId("confo(" + obj_name  +  ")");
			_object_configurations.push_back(confo_var);
			LPT_DEBUG("cout", "Variable " << confo_var << " corresponds to " << "confo(" + obj_name  +  ")")
		}
	}
	
	~BFWSF56Heuristic() = default;
	

	long compute_heuristic(const State& state, BFWSF5Node::Atomset& relevant, unsigned& num_relevant) override {
// 		std::vector<Atom> relevant_atoms;
// 		long h = this->_base_heuristic->evaluate(state, relevant_atoms);
		
// 		num_relevant = relevant_atoms.size();
		
// 		relevant = _relevant;
// 		this->build_atomset(relevant_atoms);
// 		LPT_DEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for state: " << std::endl << state << std::endl << "****************************************");
// 		return h;
		assert(0);
		return 0.0;
	}
	
	//! Return the count of how many objects offend the consecution of at least one unachived goal atom.
	unsigned compute_offending(const State& state) {
		
		std::unordered_set<VariableIdx> offending; // We'll store here which problem objects are offending some unreached goal atom.
		
		
		std::vector<unsigned> unsat_goal_indexes;
		auto conjuncts = this->_unsat_goal_atoms_heuristic.get_goal_conjuncts();
		for (unsigned i = 0; i < conjuncts.size(); ++i) {
			const fs::AtomicFormula* condition = conjuncts[i];
			
			if (!condition->interpret(state)) {  // Let's check how many objects "offend" the consecution of this goal atom
				unsat_goal_indexes.push_back(i);
			}
		}
		
		for (VariableIdx conf_var:_object_configurations) {
			ObjectIdx confo = state.getValue(conf_var);
			for (unsigned i:unsat_goal_indexes) {
				const auto& offending_to_goal_atom = _offending[i];
				
				
				if (offending_to_goal_atom.find(confo) != offending_to_goal_atom.end()) {
					// The object configuration is actually an offending one
					offending.insert(conf_var); // (the state variable IDs acts as an identifying proxy for the actual object ID)
				}
			}
		}
		
		return offending.size();
	}
	
protected:
// 	BFWSF6Node::Atomset _relevant;
	
	const fs::Conjunction* _goal_conjunction;
	
	std::vector<OffendingSet> _offending;
	
	// A vector with the variable indexes that correspond to the configuration of each object.
	std::vector<VariableIdx> _object_configurations;
};
	
	
	
	
	
	
template <>
ExitCode 
EnhancedBFWSDriver<GroundStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	
	auto model = GroundingSetup::fully_ground_model(problem);
	std::vector<OffendingSet> offending = preprocess(problem, config);
	
	
	
	SearchStats stats;
	
	BFWSConfig bfws_config(config);
	NoveltyFeaturesConfiguration feature_configuration(config);
// 	auto engine = create(config, bfws_config, feature_configuration, model, stats);
	
	
	LPT_INFO("cout", "BFWS Configuration:");
	LPT_INFO("cout", "\tBFWS Type: " << bfws_config._type);
	LPT_INFO("cout", "\tMax novelty: " << bfws_config._max_width);
	LPT_INFO("cout", "\tFeature extraction: " << feature_configuration);
	
	
	using BaseHeuristicT = gecode::SmartRPG;
	using NodeT = BFWSF6Node;
	using NodeCompareT = F6NodeComparer;
	using HeuristicEnsembleT = BFWSF56Heuristic<GroundStateModel, BaseHeuristicT>;
	using RawEngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicEnsembleT, GroundStateModel, std::shared_ptr<NodeT>, NodeCompareT>;
	using EngineT = std::unique_ptr<RawEngineT>;
	
	auto base_heuristic = std::unique_ptr<gecode::SmartRPG>(SmartEffectDriver::configure_heuristic(model.getTask(), config));
	
	auto heuristic = std::unique_ptr<HeuristicEnsembleT>(new HeuristicEnsembleT(model, bfws_config._max_width, feature_configuration, std::move(base_heuristic), std::move(offending)));
	
	auto engine = EngineT(new RawEngineT(model, *heuristic));
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicEnsembleT>(config, *heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	
	return Utils::do_search(*engine, model, out_dir, start_time, stats);
}
/*
template <typename StateModelT>
typename EnhancedBFWSDriver<StateModelT>::Engine
EnhancedBFWSDriver<StateModelT>::create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const StateModelT& model, SearchStats& stats) {
	
	// PREPROCESS
	
	
// 	_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(model, bfws_config._max_width, feature_configuration));
	auto engine = new lapkt::StlBestFirstSearch<NodeT, HeuristicT, StateModelT>(model, *_heuristic);
	
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);

	return Engine(engine);
}
*/


template <typename StateModelT>
std::vector<OffendingSet>
EnhancedBFWSDriver<StateModelT>::preprocess(const Problem& problem, const Config& config) {
	
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
// 		simplified.set_goal(conjunct->clone());
	
	using IWDriver = IteratedWidthDriver<GroundStateModel>;
	
	using PreprocessingNodeT = IWPreprocessingNode<State, ActionT>;
	using EvaluatorT = SingleNoveltyComponent<StateModelT, PreprocessingNodeT>;
	using EvaluatorPT = std::shared_ptr<EvaluatorT>;
	using OpenList = aptk::StlUnsortedFIFO<PreprocessingNodeT, EvaluatorT>;
	using BaseAlgoT = lapkt::AllSolutionsBreadthFirstSearch<PreprocessingNodeT, StateModelT, OpenList>;
	using OpenListT = aptk::StlUnsortedFIFO<PreprocessingNodeT, EvaluatorT>;
	
	auto model = GroundingSetup::fully_ground_model(simplified);
	
	EvaluatorPT evaluator = std::make_shared<EvaluatorT>(model, k, feature_configuration);
	
	BaseAlgoT iw_algorithm(model, OpenListT(evaluator), goal_conjuncts, sc_conjuncts);
// 		lapkt::events::subscribe(*_algorithm, _handlers);
	
	std::vector<ActionIdx> plan;
	LPT_INFO("cout", "PREPROCESSING: Starting IW(2) search...");
	bool solved = iw_algorithm.search(simplified.getInitialState(), plan);
	if (!solved) throw std::runtime_error("PREPROCESSING - IW(2) preprocessing did not yield a solution for all subgoals");

	LPT_INFO("cout", "PREPROCESSING: Retrieving relevant atoms...");
	auto relevant = iw_algorithm.retrieve_relevant_atom_sets();
	
	LPT_INFO("cout", "PREPROCESSING: Computing offending configuration sets...");
	std::vector<OffendingSet> offending = iw_algorithm.compute_offending_configurations();
	
	std::cout << "PREPROCESSING - RELEVANT ATOMS ARE: " << std::endl;
	for (const auto& atom:relevant) {
		std::cout << atom << ", ";
	}
	std::cout << std::endl;
	
	return offending;
// 	std::cout << "NUMBER OF SOLVED SUBGOALS: " << solved << "/" << original_goal->getConjuncts().size() << std::endl;
}

} } // namespaces
