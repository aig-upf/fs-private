
#include <unordered_set>

#include <search/drivers/bfws/enhanced_bfws.hxx>
#include <search/drivers/bfws/preprocessing.hxx>
#include <search/drivers/bfws/ctmp_features.hxx>
#include <search/drivers/iterated_width.hxx>
#include <search/utils.hxx>
#include <utils/utils.hxx>
#include <utils/external.hxx>
#include <utils/printers/feature_set.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <applicability/gecode_analyzer.hxx>
#include <actions/grounding.hxx>

using namespace fs0;
namespace fs = fs0::language::fstrips;



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
	std::cout << "INCLUDING TRAJECTORY: " << config.include_trajectory() << std::endl;
	// Add all state variables
	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		if (config.include_trajectory() || var != traj) {
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
		_num_offending = ensemble.compute_offending(*this, picked_offending_object, _h);
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
		_custom_heuristic(_problem.getGoalConditions()),
		_per_goal_rel_var(_unsat_goal_atoms_heuristic.get_goal_conjuncts().size(), -1)
	{
		const ProblemInfo& info = ProblemInfo::getInstance();
		TypeIdx obj_t = info.getTypeId("object_id");
		for (ObjectIdx obj_id:info.getTypeObjects(obj_t)) {
			std::string obj_name = info.deduceObjectName(obj_id, obj_t);
			VariableIdx confo_var = info.getVariableId("confo(" + obj_name  +  ")");
			_object_configurations.push_back(confo_var);
		}
		
		
		auto conjuncts = this->_unsat_goal_atoms_heuristic.get_goal_conjuncts();
		for (unsigned i = 0; i < conjuncts.size(); ++i) {
			const fs::AtomicFormula* condition = conjuncts[i];
			
			auto scope = fs::ScopeUtils::computeDirectScope(condition);
			if (scope.size() != 1) throw std::runtime_error("Unsupported goal type");
			_per_goal_rel_var[i] = scope[0];
		}
	}
	
	~BFWSF6Heuristic() = default;

	inline unsigned max_novelty() { return _max_novelty; }
	
	unsigned get_unachieved(const State& state) {
		return _unsat_goal_atoms_heuristic.evaluate(state);
	}
	
	
	//! Return the count of how many objects offend the consecution of at least one unachived goal atom.
	template <typename NodeT>
	unsigned compute_offending(const NodeT& node, ObjectIdx& picked_offending_object, unsigned& h) {
		const ProblemInfo& info = ProblemInfo::getInstance();
		const Problem& problem = Problem::getInstance();
		
		const State& state = node.state;
		
		std::unordered_set<VariableIdx> offending; // We'll store here which problem objects are offending some unreached goal atom.
// 		std::vector<VariableIdx> offending;
		
		
		// COMPUTE WHICH ATOM GOALS HAVE NOT BEEN REACHED YET
		std::vector<unsigned> unsat_goal_indexes;
		auto conjuncts = _unsat_goal_atoms_heuristic.get_goal_conjuncts();
		for (unsigned i = 0; i < conjuncts.size(); ++i) {
			const fs::AtomicFormula* condition = conjuncts[i];
			
			if (!condition->interpret(state)) {  // Let's check how many objects "offend" the consecution of this goal atom
				unsat_goal_indexes.push_back(i);
			}
		}
		
		std::vector<bool> is_path_to_goal_atom_clear(_unsat_goal_atoms_heuristic.get_goal_conjuncts().size(), true);
		
		// For each object, we check whether the configuration of the object in the current state
		// offends the consecution of _at least one_ unsatisfied goal atom (e.g. of the form confo(o3)=c15)
		for (VariableIdx conf_var:_object_configurations) {
			ObjectIdx confo = state.getValue(conf_var);
			for (unsigned i:unsat_goal_indexes) {
				
				if (conf_var == _per_goal_rel_var[i]) continue; // Don't consider as offending to a goal atom the config of the object appearing on that atom itself!
				
				// 'offending_to_goal_atom' will contain all possible object configurations that offend the consecution of this particular unsatisfied goal atom
				const auto& offending_to_goal_atom = _offending[i]; 
				
				if (offending_to_goal_atom.find(confo) != offending_to_goal_atom.end()) {
					// The object configuration is actually an offending one
					offending.insert(conf_var); // (the state variable IDs acts as an identifying proxy for the actual object ID)
					is_path_to_goal_atom_clear[i] = false;
				}
			}
		}
		/*
		std::cout << "Offending objects: " << std::endl;
		for (auto v:offending) {
			std::cout << info.getVariableName(v) << std::endl;
		}
		std::cout << std::endl;
		*/
		
		h = _custom_heuristic.evaluate(state, is_path_to_goal_atom_clear);
		
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
	
	std::vector<VariableIdx> _per_goal_rel_var;
	
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
	
	
	std::vector<OffendingSet> offending = EBFWSPreprocessor::preprocess(problem, config, analyzer);
	
	
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



} } // namespaces
