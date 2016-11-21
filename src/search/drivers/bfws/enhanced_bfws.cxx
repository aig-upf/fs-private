
#include <unordered_set>

#include <search/drivers/bfws/enhanced_bfws.hxx>
#include <search/drivers/bfws/ctmp_features.hxx>
#include <search/drivers/iterated_width.hxx>
#include <search/utils.hxx>
#include <utils/utils.hxx>
#include <utils/external.hxx>
#include <utils/printers/feature_set.hxx>
#include <languages/fstrips/language.hxx>
#include <applicability/gecode_analyzer.hxx>
#include <actions/grounding.hxx>

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


	

std::vector<ValuesTuple::ValueIndex>
CTMPNoveltyEvaluator::compute_valuation(const State& state) const {
// 	LPT_INFO("novelty-evaluations", "Evaluating state " << state);
	std::vector<ValuesTuple::ValueIndex> values;
	
	values.reserve(_features.size());
	for (const auto& feature:_features) {
		values.push_back(feature->evaluate(state));
		// 		LPT_INFO("novelty-evaluations", "\t" << _featureMap.feature(k) << ": " << values[k]);
	}

	return values;
// 	LPT_DEBUG("heuristic", "Feature evaluation: " << std::endl << print::feature_set(varnames, values));
}



CTMPNoveltyEvaluator::CTMPNoveltyEvaluator(const Problem& problem, unsigned novelty_bound, const NoveltyFeaturesConfiguration& feature_configuration, bool check_overlaps, bool placeable, bool graspable) :
	Base(novelty_bound),
	_features(selectFeatures(problem, feature_configuration, check_overlaps, placeable, graspable))
{}

CTMPNoveltyEvaluator::CTMPNoveltyEvaluator(const CTMPNoveltyEvaluator& other)
	: Base(other), _features(clone_features(other._features))
{}

CTMPNoveltyEvaluator::FeatureSet
CTMPNoveltyEvaluator::clone_features(const FeatureSet& features) {
	FeatureSet cloned;
	for (unsigned i = 0; i < features.size(); ++i) {
		cloned.push_back(std::unique_ptr<NoveltyFeature>(features[i]->clone()));
	}
	return cloned;
}

CTMPNoveltyEvaluator::FeatureSet
CTMPNoveltyEvaluator::selectFeatures(const Problem& problem, const NoveltyFeaturesConfiguration& config, bool check_overlaps, bool placeable, bool graspable) {
	const ProblemInfo& info = ProblemInfo::getInstance();

	FeatureSet features;
	
	VariableIdx traj = info.getVariableId("traj(rob)");
// 	traj = -1;
	
	// Add all state variables
	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		if (var != traj) {
			features.push_back(std::unique_ptr<NoveltyFeature>(new StateVariableFeature(var)));
		}
	}
	
	// Add now some domain-dependent features:
	/*
	// For each movable object o, consider the value of "@graspable(confb(rob), confa(rob), confo(o))" as a novelty feature
	TypeIdx obj_t = info.getTypeId("object_id");
	unsigned graspable_id = info.getSymbolId("@graspable");
	unsigned confb_id = info.getSymbolId("confb");
	unsigned confa_id = info.getSymbolId("confa");
	unsigned confo_id = info.getSymbolId("confo");
	ObjectIdx rob_id = info.getObjectId("rob");
	
	
	fs::FluentHeadedNestedTerm confb_rob(confb_id, { new fs::Constant(rob_id) });
	fs::FluentHeadedNestedTerm confa_rob(confa_id, { new fs::Constant(rob_id) });
	
	
	Binding empty_binding;
	
	for (ObjectIdx obj_id:info.getTypeObjects(obj_t)) {
		fs::FluentHeadedNestedTerm confo_o(confo_id, { new fs::Constant(obj_id) });
		
		auto feature_term = new fs::UserDefinedStaticTerm(graspable_id, {
			confb_rob.bind(empty_binding, info),
			confa_rob.bind(empty_binding, info),
			confo_o.bind(empty_binding, info),
		});
		
		LPT_INFO("cout", "Adding Term-based Novelty feature: " << *feature_term);
		features.push_back(std::unique_ptr<ArbitraryTermFeature>(new ArbitraryTermFeature(feature_term)));
	}
	*/
	
	if (placeable)
		features.push_back(std::unique_ptr<PlaceableFeature>(new PlaceableFeature(check_overlaps, problem.getGoalConditions())));
	
	if (graspable)
		features.push_back(std::unique_ptr<GraspableFeature>(new GraspableFeature));
	
	
// 	features.push_back(std::unique_ptr<GlobalRobotConfFeature>(new GlobalRobotConfFeature));
	
	LPT_INFO("cout", "Number of features from which state novelty will be computed: " << features.size());
	return features;
}


// template <typename PrunerT = NullPruner>
class BFWSF6Node {
public:
	using ptr_t = std::shared_ptr<BFWSF6Node>;
	
	State state;
	GroundAction::IdType action;
	
	FeatureValuation feature_valuation;
	
	unsigned type; // The type of novelty
	
	ptr_t parent;

	//! Accummulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;
	
	//! Number of unachieved goals (#g)
	unsigned unachieved;
	
	//! The number of objects in an offending configuration
	unsigned _num_offending;
	
	ObjectIdx picked_offending_object; // The object that we picked and that was offending
	
	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
// 	enum class REACHED_STATUS : unsigned char {IRRELEVANT_ATOM, UNREACHED, REACHED};
// 	using Atomset = std::vector<REACHED_STATUS>;
// 	Atomset _reached_idx;
// 	unsigned _num_relaxed_achieved;
	
	//!
	unsigned _h;
	
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
		state(std::move(_state)),
		action(action_),
		feature_valuation(),
		parent(parent_),
		g(parent ? parent->g+1 : 0),
		novelty(std::numeric_limits<unsigned>::max()),
		unachieved(std::numeric_limits<unsigned>::max()),
		_num_offending(std::numeric_limits<unsigned>::max()),
// 		_num_relaxed_achieved(0),
		picked_offending_object(-1),
		_h(std::numeric_limits<unsigned>::max())
	{}

	bool has_parent() const { return parent != nullptr; }
	
	//! Required for the interface of some algorithms that might prioritise helpful actions.
	bool is_helpful() const { return false; }

	bool operator==( const BFWSF6Node& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const BFWSF6Node& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		std::string hval = _h == std::numeric_limits<unsigned>::max() ? "?" : std::to_string(_h);
		return os << "{@ = " << this << ", s = " << state << ", g = " << g << ", w = " << novelty << ", #g=" << unachieved << ", #off=" << _num_offending << ", p = " << picked_offending_object << ", h = " << hval << ", parent = " << parent << "}";
	}
	
	/*
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
	*/
	
	template <typename Heuristic>
	void evaluate_with( Heuristic& ensemble ) {
		unachieved = ensemble.get_unachieved(this->state);
		_num_offending = ensemble.compute_offending(*this, picked_offending_object);
		_h = ensemble.compute_heuristic(state);
		novelty = ensemble.novelty(*this, unachieved, _h, _num_offending);
		if (novelty > ensemble.max_novelty()) {
			novelty = std::numeric_limits<unsigned>::max();
		}
		
		/*
		if (this->parent->_num_offending == _num_offending + 2) {
			// It must be that we picked an offending configuration
			const GroundAction
			if (true) throw std::runtime_error("WRONG ASSUMPTION");
			
			
			
		}
		*/
	}
	
	void inherit_heuristic_estimate() {
		throw std::runtime_error("SHOULDN'T BE HERE");
	}
	
	//! What to do when an 'other' node is found during the search while 'this' node is already in
	//! the open list
	void update_in_open_list(ptr_t other) {
		return; // We want no  updates ATM, since it messes with width considerations
	}

	bool dead_end() const { return false; }
};


// W only
struct F6NodeComparer0 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;

		return n1->g > n2->g;
	}
};

// W, H, #OFF, #G
struct F6NodeComparer1 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;

		return n1->g > n2->g;
	}
};

// W, #G, H, #OFF
struct F6NodeComparer2 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;

		return n1->g > n2->g;
	}
};

// W, #G, #OFF, H
struct F6NodeComparer3 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;

		return n1->g > n2->g;
	}
};

// W, #OFF, #G, H
struct F6NodeComparer4 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;

		return n1->g > n2->g;
	}
};

// W, #OFF, H, #G
struct F6NodeComparer5 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;

		return n1->g > n2->g;
	}
};


// #OFF, W, #G, H
struct F6NodeComparer6 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;

		return n1->g > n2->g;
	}
};

// #G, W, #OFF, H
struct F6NodeComparer7 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;

		return n1->g > n2->g;
	}
};

// #OFF, #G, W, H
struct F6NodeComparer8 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;
		
		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;

		return n1->g > n2->g;
	}
};


// #G, #OFF, W, H
struct F6NodeComparer9 {
	using NodePtrT = std::shared_ptr<BFWSF6Node>;
	bool operator()(const NodePtrT& n1, const NodePtrT& n2) const {

		if (n1->unachieved > n2->unachieved) return true;
		if (n1->unachieved < n2->unachieved) return false;
		
		if (n1->_num_offending > n2->_num_offending) return true;
		if (n1->_num_offending < n2->_num_offending) return false;

		if (n1->novelty > n2->novelty) return true;
		if (n1->novelty < n2->novelty) return false;
		
		if (n1->_h > n2->_h) return true;
		if (n1->_h < n2->_h) return false;

		return n1->g > n2->g;
	}
};

//! For the problem at hand, 'unachieved' will typically range 0-100, 'offending': 0-100, heuristic: 0-200
inline unsigned _index(unsigned unachieved, unsigned heuristic, unsigned offending) {
	return (heuristic<<16) | (offending<<8) |  unachieved;
}


// No index
struct NoveltyIndexer0 {
	unsigned operator()(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return 0;
	}

	std::tuple<unsigned, unsigned, unsigned> relevant(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return std::make_tuple(0, 0, 0);
	}
};

// #g
struct NoveltyIndexer1 {
	unsigned operator()(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return unachieved;
	}

	std::tuple<unsigned, unsigned, unsigned> relevant(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return std::make_tuple(unachieved, 0, 0);
	}
};

// #g, #off
struct NoveltyIndexer2 {
	unsigned operator()(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return _index(unachieved, 0, offending);
	}

	std::tuple<unsigned, unsigned, unsigned> relevant(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return std::make_tuple(unachieved, 0, offending);
	}
};


// h, #g
struct NoveltyIndexer3 {
	unsigned operator()(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return _index(unachieved, heuristic, 0);
	}

	std::tuple<unsigned, unsigned, unsigned> relevant(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return std::make_tuple(unachieved, heuristic, 0);
	}
};

// h, #g, #off
struct NoveltyIndexer4 {
	unsigned operator()(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return _index(unachieved, heuristic, offending);
	}

	std::tuple<unsigned, unsigned, unsigned> relevant(unsigned unachieved, unsigned heuristic, unsigned offending) const {
		return std::make_tuple(unachieved, heuristic, offending);
	}
};

template <typename StateModelT, typename BaseHeuristicT, typename NoveltyIndexerT>
class BFWSF6Heuristic {
public:
	BFWSF6Heuristic(const StateModelT& model,
                 unsigned max_novelty,
                 const NoveltyFeaturesConfiguration& feature_configuration,
                 const CTMPNoveltyEvaluator& novelty_evaluator,
//                  std::unique_ptr<BaseHeuristicT>&& heuristic,
                 std::vector<OffendingSet>&& offending) :
		_problem(model.getTask()),
		_feature_configuration(feature_configuration),
		_max_novelty(max_novelty), 
		_ctmp_novelty_evaluators(),
// 		_base_heuristic(std::move(heuristic)),
		_unsat_goal_atoms_heuristic(model),
		_offending(std::move(offending)),
		_base_evaluator(novelty_evaluator),
		_custom_heuristic(_problem.getGoalConditions())
	{
		const ProblemInfo& info = ProblemInfo::getInstance();
		TypeIdx obj_t = info.getTypeId("object_id");
		for (ObjectIdx obj_id:info.getTypeObjects(obj_t)) {
			std::string obj_name = info.deduceObjectName(obj_id, obj_t);
			VariableIdx confo_var = info.getVariableId("confo(" + obj_name  +  ")");
			_object_configurations.push_back(confo_var);
		}
	}
	
	~BFWSF6Heuristic() = default;

	inline unsigned max_novelty() { return _max_novelty; }
	
	unsigned get_unachieved(const State& state) {
		return _unsat_goal_atoms_heuristic.evaluate(state);
	}
	
	unsigned compute_heuristic(const State& state) {
		return _custom_heuristic.evaluate(state);
	}

	
	//! Return the count of how many objects offend the consecution of at least one unachived goal atom.
	template <typename NodeT>
	unsigned compute_offending(const NodeT& node, ObjectIdx& picked_offending_object) {
		const ProblemInfo& info = ProblemInfo::getInstance();
		const Problem& problem = Problem::getInstance();
		
		const State& state = node.state;
		
		std::unordered_set<VariableIdx> offending; // We'll store here which problem objects are offending some unreached goal atom.
		
		
		std::vector<unsigned> unsat_goal_indexes;
		auto conjuncts = this->_unsat_goal_atoms_heuristic.get_goal_conjuncts();
		for (unsigned i = 0; i < conjuncts.size(); ++i) {
			const fs::AtomicFormula* condition = conjuncts[i];
			
			if (!condition->interpret(state)) {  // Let's check how many objects "offend" the consecution of this goal atom
				unsat_goal_indexes.push_back(i);
			}
		}
		
		// For each object, we check whether the configuration of the object in the current state
		// offends the consecution of _at least one_ unsatisfied goal atom (e.g. of the form confo(o3)=c15)
		for (VariableIdx conf_var:_object_configurations) {
			ObjectIdx confo = state.getValue(conf_var);
			for (unsigned i:unsat_goal_indexes) {
				
				// 'offending_to_goal_atom' will contain all possible object configurations that offend the consecution of this particular unsatisfied goal atom
				const auto& offending_to_goal_atom = _offending[i]; 
				
				if (offending_to_goal_atom.find(confo) != offending_to_goal_atom.end()) {
					// The object configuration is actually an offending one
					offending.insert(conf_var); // (the state variable IDs acts as an identifying proxy for the actual object ID)
				}
			}
		}
		
		unsigned num_offending = 2 * offending.size();
		
// 		std::cout << "Num offending #1: " << num_offending << "   " << state.hash() <<std::endl;
		
		if (node.action == GroundAction::invalid_action_id || node.parent == nullptr) return num_offending;
		
		picked_offending_object = -1;
		const GroundAction& action = *(problem.getGroundActions().at(node.action));
		const ValueTuple& values = action.getBinding().get_full_binding();
		
// 		bool yes = (node.parent->_num_offending == num_offending + 2);
// 		if (yes) std::cout << "!!Num offending #2: " << num_offending << std::endl;
// 		if (yes) std::cout << "!!Num offending #1.1: " << node.parent->_num_offending << ", " << num_offending << std::endl;
		
		if (action.getName() == "grasp-object" && (node.parent->_num_offending == num_offending + 2)) {
			// It must be that we picked an offending configuration
			
// 			if (yes) std::cout << "Num offending #3: " << num_offending << std::endl;
			
			assert(values.size()==1); // The action has exactly one parameter
			picked_offending_object = values[0];
			
			VariableIdx v_holding = info.getVariableId("holding()");
			if (state.getValue(v_holding) != picked_offending_object) throw std::runtime_error("WRONG ASSUMPTION");
			
			num_offending += 1;
			
		} else {
			ObjectIdx previously_picked =  node.parent->picked_offending_object;
			
// 			std::cout << "Num offending #4: " << num_offending << std::endl;
			
			if (previously_picked != -1) { // We were carrying a picked object in the parent
				
// 				std::cout << "Num offending #5: " << num_offending << std::endl;
				
				if (action.getName() == "place-object") { // We're placing the previously-picked offending object
					// We must be placing the ex-offending object
					if (values.size() != 1 ||  values[0] != previously_picked) throw std::runtime_error("WRONG ASSUMPTION");
					
				} else {
					// Else we're still carrying a previously picked offending object, so we add a +1 to the num_offending
					num_offending += 1;
					picked_offending_object = previously_picked;
				}
			}
		}
		
// 		std::cout << "Num offending FINAL: " << num_offending << "   " << state.hash() <<std::endl;
		return num_offending;
	}
	
	//! Compute the novelty of the state wrt all the states with the same heuristic value.
	template <typename NodeT>
	unsigned novelty(NodeT& node, unsigned unachieved, unsigned heuristic, unsigned offending) {
		auto ind = _indexer(unachieved, heuristic, offending);
#ifdef DEBUG
		// Let's make sure that either it's the first time we see this index, or, if was already there, 
		// it corresponds to the same combination of <unachieved, heuristic, offending>
		auto tuple = _indexer.relevant(unachieved, heuristic, offending);
		auto __it =  __novelty_idx_values.find(ind);
		if (__it == __novelty_idx_values.end()) {
			__novelty_idx_values.insert(std::make_pair(ind, tuple));
		} else {
			assert(__it->second == tuple);
		}
#endif
		auto it = _ctmp_novelty_evaluators.find(ind);
		if (it == _ctmp_novelty_evaluators.end()) {
			auto inserted = _ctmp_novelty_evaluators.insert(std::make_pair(ind, _base_evaluator));
			it = inserted.first;
		}
		
		CTMPNoveltyEvaluator& evaluator = it->second;
		
		node.type = ind;
		node.feature_valuation = evaluator.compute_valuation(node.state);
		return evaluator.evaluate(node);
	}
	
	//! For the problem at hand, 'unachieved' will typically range 0-100, 'offending': 0-100, heuristic: 0-200
	static inline unsigned index(unsigned unachieved, unsigned heuristic, unsigned offending) {
		return (heuristic<<16) | (offending<<8) |  unachieved;
	}

protected:
// 	BFWSF6Node::Atomset _relevant;
	
#ifdef DEBUG
	// Just for sanity check purposes
	std::map<unsigned, std::tuple<unsigned,unsigned,unsigned>> __novelty_idx_values;
#endif
	
	const Problem& _problem;
	
	const NoveltyFeaturesConfiguration& _feature_configuration;
	
	unsigned _max_novelty;

	//! We have one different novelty evaluators for each actual heuristic value that a node might have.
	std::unordered_map<unsigned, CTMPNoveltyEvaluator> _ctmp_novelty_evaluators;
	
// 	std::unique_ptr<BaseHeuristicT> _base_heuristic;
	
	//! An UnsatisfiedGoalAtomsHeuristic to count the number of unsatisfied goals
	UnsatisfiedGoalAtomsHeuristic<StateModelT> _unsat_goal_atoms_heuristic;
	
	const fs::Conjunction* _goal_conjunction;
	
	std::vector<OffendingSet> _offending;
	
	// A vector with the variable indexes that correspond to the configuration of each object.
	std::vector<VariableIdx> _object_configurations;
	
	// We keep a base evaluator to be cloned each time a new one is needed, so that there's no need
	// to perform all the feature selection, etc. anew.
	const CTMPNoveltyEvaluator& _base_evaluator;
	
	CustomHeuristic _custom_heuristic;
	
	NoveltyIndexerT _indexer;
};


// An ad-hoc node pruner that prunes if the novelty of the given node is larger than 2
template <typename NodePT>
struct W2Pruner {
	static bool inline prune(const NodePT& node) { 
		if(node->novelty < 1) throw std::runtime_error("SHOULDN'T BE HAPPENING");
		return node->novelty > 2;
	}
};

ExitCode 
EnhancedBFWSDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	std::string order = config.getOption<std::string>("ebfws.order");
	if (order == "0") {
// 		return do_search<F6NodeComparer0>(problem, config, out_dir, start_time);
	} else if (order == "1") {
// 		return do_search<F6NodeComparer1>(problem, config, out_dir, start_time);
	} else if (order == "2") {
// 		return do_search<F6NodeComparer2>(problem, config, out_dir, start_time);
	} else if (order == "3") {
// 		return do_search<F6NodeComparer3>(problem, config, out_dir, start_time);
	} else if (order == "4") {
		return do_search<F6NodeComparer4>(problem, config, out_dir, start_time);
	} else if (order == "5") {
		return do_search<F6NodeComparer5>(problem, config, out_dir, start_time);
	} else if (order == "6") {
// 		return do_search<F6NodeComparer6>(problem, config, out_dir, start_time);
	} else if (order == "7") {
		return do_search<F6NodeComparer7>(problem, config, out_dir, start_time);
	} else if (order == "8") {
		return do_search<F6NodeComparer8>(problem, config, out_dir, start_time);
	} else if (order == "9") {
// 		return do_search<F6NodeComparer9>(problem, config, out_dir, start_time);
	}
	
	throw std::runtime_error("Invalid value " + order + " for configuration option \"ebfws.order\"");
}

template <typename NodeCompareT>
ExitCode 
EnhancedBFWSDriver::do_search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	std::string novelty_t = config.getOption<std::string>("ebfws.novelty_type");
	if (novelty_t == "0") {
		return do_search<NodeCompareT, NoveltyIndexer0>(problem, config, out_dir, start_time);
	} else if (novelty_t == "1") {
// 		return do_search<NodeCompareT, NoveltyIndexer1>(problem, config, out_dir, start_time);
	} else if (novelty_t == "2") {
// 		return do_search<NodeCompareT, NoveltyIndexer2>(problem, config, out_dir, start_time);
	} else if (novelty_t == "3") {
		return do_search<NodeCompareT, NoveltyIndexer3>(problem, config, out_dir, start_time);
	} else if (novelty_t == "4") {
		return do_search<NodeCompareT, NoveltyIndexer4>(problem, config, out_dir, start_time);
	}
	throw std::runtime_error("Invalid value " + novelty_t + " for configuration option \"ebfws.novelty_type\"");
}

template <typename NodeCompareT, typename NoveltyIndexerT>
ExitCode 
EnhancedBFWSDriver::do_search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	
	std::string pruning_t = config.getOption<std::string>("ebfws.pruning", "none");
	if (pruning_t == "w2") {
		return do_search_1<NodeCompareT, NoveltyIndexerT, W2Pruner<std::shared_ptr<BFWSF6Node>>, aptk::NullClosedList<BFWSF6Node>>(problem, config, out_dir, start_time);
	} else if (pruning_t == "w2_closed") {
		return do_search_1<NodeCompareT, NoveltyIndexerT, W2Pruner<std::shared_ptr<BFWSF6Node>>, aptk::StlUnorderedMapClosedList<BFWSF6Node>>(problem, config, out_dir, start_time);
	}
	// Otherwise, we want no pruning at all
	return do_search_1<NodeCompareT, NoveltyIndexerT, lapkt::NullPruner<std::shared_ptr<BFWSF6Node>>, aptk::StlUnorderedMapClosedList<BFWSF6Node>>(problem, config, out_dir, start_time);
}

template <typename NodeCompareT, typename NoveltyIndexerT, typename PrunerT, typename ClosedListT>
ExitCode
EnhancedBFWSDriver::do_search_1(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	
	BasicApplicabilityAnalyzer* analyzer = nullptr;
	
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	
	const auto& actions = problem.getGroundActions();
	const auto& tuple_idx =  problem.get_tuple_index();
	
	if (config.getOption<std::string>("applicability", "basic") == "gecode") {
		analyzer = new GecodeApplicabilityAnalyzer(actions, tuple_idx);
	} else {
		analyzer = new BasicApplicabilityAnalyzer(actions, tuple_idx);
	}
	analyzer->build();
	
	NoveltyFeaturesConfiguration feature_configuration(config);
	unsigned max_width = config.getOption<int>("width.max");
	
	
	std::vector<OffendingSet> offending = preprocess(problem, config, analyzer);
	
	
	LPT_INFO("cout", "CTMP-BFWS Configuration:");
	LPT_INFO("cout", "\tMax width: " << max_width);
	
	// Create here one instance to be copied around, so that no need to keep reanalysing which features are relevant
	CTMPNoveltyEvaluator base_novelty_evaluator(problem, max_width, feature_configuration, true, true, true);
	
	
	GroundStateModel model(problem, analyzer, true); 
	
	using BaseHeuristicT = gecode::SmartRPG;
	using NodeT = BFWSF6Node;
	using NodePT = std::shared_ptr<NodeT>;
	

	using HeuristicEnsembleT = BFWSF6Heuristic<GroundStateModel, BaseHeuristicT, NoveltyIndexerT>;
// 	using OpenListT = lapkt::BaseSortedOpenList<NodeT, HeuristicEnsembleT, NodePT, std::vector<NodePT>, NodeCompareT>;
	using OpenListT = lapkt::StlSortedOpenList<NodeT, HeuristicEnsembleT, NodePT, std::vector<NodePT>, NodeCompareT>;

// 	auto base_heuristic = std::unique_ptr<gecode::SmartRPG>(SmartEffectDriver::configure_heuristic(model.getTask(), config));
// 	std::unique_ptr<gecode::SmartRPG> nullheuristic;

	using RawEngineT = lapkt::StlBestFirstSearch<NodeT, HeuristicEnsembleT, GroundStateModel, NodePT, NodeCompareT, OpenListT, ClosedListT, PrunerT>;
	using EngineT = std::unique_ptr<RawEngineT>;
	auto heuristic = std::unique_ptr<HeuristicEnsembleT>(new HeuristicEnsembleT(model, max_width, feature_configuration, base_novelty_evaluator, std::move(offending)));
	
	auto engine = EngineT(new RawEngineT(model, *heuristic));
	
	SearchStats stats;
	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicEnsembleT>(config, *heuristic, stats, _handlers);
	lapkt::events::subscribe(*engine, _handlers);
	
	
	auto res = Utils::do_search(*engine, model, out_dir, start_time, stats);
	
	delete analyzer;
	return res;
}

std::vector<OffendingSet>
EnhancedBFWSDriver::preprocess(const Problem& problem, const Config& config, BasicApplicabilityAnalyzer* analyzer) {
	
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
	
	using PreprocessingNodeT = IWPreprocessingNode<State, ActionT>;
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
