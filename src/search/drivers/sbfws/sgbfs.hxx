
#pragma once

#include <search/drivers/sbfws/iw_run.hxx>
#include <search/drivers/registry.hxx>
#include <search/drivers/setups.hxx>
#include <search/drivers/sbfws/base.hxx>
#include "hff_run.hxx"
#include <heuristics/unsat_goal_atoms/unsat_goal_atoms.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>
#include <heuristics/novelty/features.hxx>
#include <lapkt/components/open_lists.hxx>


namespace fs0 { namespace bfws {


//! Prioritize nodes with lower number of _un_achieved subgoals. Break ties with g.
template <typename NodePT>
struct unachieved_subgoals_comparer {
	bool operator()(const NodePT& n1, const NodePT& n2) const {
		if (n1->unachieved_subgoals > n2->unachieved_subgoals) return true;
		if (n1->unachieved_subgoals < n2->unachieved_subgoals) return false;
		if (n1->g > n2->g) return true;
		if (n1->g < n2->g) return false;
		return n1->_gen_order > n2->_gen_order;
	}
};

// ! Comparer taking into account #g and novelty
template <typename NodePT>
struct novelty_comparer {
	bool operator()(const NodePT& n1, const NodePT& n2) const {
		if (n1->w_g_num > n2->w_g_num) return true;
		if (n1->w_g_num < n2->w_g_num) return false;
		if (n1->unachieved_subgoals > n2->unachieved_subgoals) return true;
		if (n1->unachieved_subgoals < n2->unachieved_subgoals) return false;
		if (n1->g > n2->g) return true;
		if (n1->g < n2->g) return false;
		return n1->_gen_order > n2->_gen_order;
	}
};

enum class Novelty { Unknown, One, GTOne, Two, GTTwo};

//! The node type we'll use for the Simulated BFWS search, parametrized by type of state and action action
template <typename StateT, typename ActionT>
class LazyBFWSNode {
public:
	using ptr_t = std::shared_ptr<LazyBFWSNode<StateT, ActionT>>;
	using action_t = typename ActionT::IdType;
	
	//! The state corresponding to the search node
	StateT state;

	//! The action that led to the state in this search node
	action_t action;

	//! The parent search node
	ptr_t parent;

	//! Accummulated cost
	unsigned g;

	//! The number of unachieved goals (#g)
	unsigned unachieved_subgoals;

	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
	RelevantAtomSet _relevant_atoms;

	bool _processed;
	
	//! Whether a simulation has already been run from this node
	bool _simulated;

	//! The generation order, uniquely identifies the node
	unsigned long _gen_order;

	//! The novelty w_{#g} of the state
	Novelty w_g;
	
	//! The novelty w_{#g,#r} of the state
	Novelty w_gr;
	
	
	LazyBFWSNode() = delete;
	~LazyBFWSNode() = default;

	LazyBFWSNode(const LazyBFWSNode&) = delete;
	LazyBFWSNode(LazyBFWSNode&&) = delete;
	LazyBFWSNode& operator=(const LazyBFWSNode&) = delete;
	LazyBFWSNode& operator=(LazyBFWSNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	LazyBFWSNode(const StateT& s, unsigned long gen_order) : LazyBFWSNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	LazyBFWSNode(StateT&& _state, action_t action_, ptr_t parent_, unsigned long gen_order) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0),
		unachieved_subgoals(std::numeric_limits<unsigned>::max()),
		_relevant_atoms(nullptr),
		_processed(false),
		_simulated(false),
		_gen_order(gen_order),
		w_g(Novelty::Unknown),
		w_gr(Novelty::Unknown)
	{}
	
	
	inline std::string print_novelty(const Novelty& novelty) const { 
		if (novelty == Novelty::Unknown) return "=?";
		if (novelty == Novelty::One) return "=1";
		if (novelty == Novelty::GTOne) return ">1";
		if (novelty == Novelty::Two) return "=2";
		if (novelty == Novelty::GTTwo) return ">2";
		throw std::runtime_error("Unknown novelty value");
	}

	bool has_parent() const { return parent != nullptr; }

	bool operator==( const LazyBFWSNode<StateT, ActionT>& o ) const { return state == o.state; }

	bool dead_end() const { return false; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const LazyBFWSNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		return os << "{@ = " << this << ", s = " << state << ", g = " << g << ", w_g" << print_novelty(w_g) <<  ", w_gr" << print_novelty(w_gr) << ", #g=" << unachieved_subgoals << ", #r=" << _relevant_atoms.num_reached() << ", parent = " << parent << "}";
	}


	//! Return the (possibly cached) set R of relevant atoms corresponding to this node.
	//! This might trigger the recursive computation from the parent node.
	template <typename HeuristicT>
	const RelevantAtomSet& get_relevant_atoms(HeuristicT& heuristic) {
		if (!_relevant_atoms.valid()) {
			heuristic.update_relevant_atoms(*this);
		}
		return _relevant_atoms;
	}

	bool decreases_unachieved_subgoals() const {
		return (!has_parent() || unachieved_subgoals < parent->unachieved_subgoals);
	}
};



template <typename StateModelT, typename NoveltyIndexerT>
class LazyBFWSHeuristic {
public:
	using FeatureSetT = lapkt::novelty::StraightBinaryFeatureSetEvaluator<State>;
	using NoveltyEvaluatorMapT = std::unordered_map<long, FSBinaryNoveltyEvaluatorI*>;
	using ActionT = typename StateModelT::ActionType;
	using IWNodeT = IWRunNode<State, ActionT>;
	using IWAlgorithm = IWRun<IWNodeT, StateModelT>;
	using IWNodePT = typename IWAlgorithm::NodePT;

	using APTKFFHeuristicT = HFFRun;
	using APTKFFHeuristicPT = std::unique_ptr<APTKFFHeuristicT>;



	LazyBFWSHeuristic(const SBFWSConfig& config, const StateModelT& model, const FeatureSetT& features, const FSBinaryNoveltyEvaluatorI& search_evaluator, const FSBinaryNoveltyEvaluatorI& simulation_evaluator, BFWSStats& stats) :
		_model(model),
		_problem(model.getTask()),
		_featureset(features),
		_search_evaluator(search_evaluator),
		_simulation_evaluator(simulation_evaluator),
		_wg_novelty_evaluators(),
		_wgr_novelty_evaluators(),
		_unsat_goal_atoms_heuristic(_problem),
		_mark_negative_propositions(config.mark_negative_propositions),
		_complete_simulation(config.complete_simulation),
		_stats(stats),
		_aptk_rpg(nullptr),
		_use_simulation_nodes(false)
	{
		if (config.relevant_set_type == SBFWSConfig::RelevantSetType::APTK_HFF) { // Setup the HFF heuristic from LAPKT
			_aptk_rpg = APTKFFHeuristicPT(APTKFFHeuristicT::create(_problem, true));
		} else if (config.relevant_set_type == SBFWSConfig::RelevantSetType::Macro) {
			_use_simulation_nodes = true;
		}
		
		// Else we'll simply use simulators
	}

	~LazyBFWSHeuristic() {
		for (auto& p:_wg_novelty_evaluators) delete p.second;
		for (auto& p:_wgr_novelty_evaluators) delete p.second;
	};

	
	template <typename NodeT>
	unsigned evaluate_wg1(NodeT& node) {
		unsigned type = node.unachieved_subgoals;
// 		bool has_parent = node.has_parent() && (node.parent->w_g != Novelty::Unknown);
		bool has_parent = node.has_parent();
		unsigned ptype = has_parent ? node.parent->unachieved_subgoals : 0; // If the node has no parent, this value doesn't matter.
		unsigned nov = evaluate_novelty(node, _wg_novelty_evaluators, 1, has_parent, type, ptype);
		assert(node.w_g == Novelty::Unknown);
		node.w_g = (nov == 1) ? Novelty::One : Novelty::GTOne;
		return nov;

	}
	
	template <typename NodeT>
	unsigned evaluate_wg2(NodeT& node) {
		unsigned type = node.unachieved_subgoals;
// 		bool has_parent = node.has_parent() && (node.parent->w_g == Novelty::Two || node.parent->w_g == Novelty::GTTwo); // i.e. the state has been evaluated on the novelty-two tables
		bool has_parent = node.has_parent();
		unsigned ptype = has_parent ? node.parent->unachieved_subgoals : 0; // If the node has no parent, this value doesn't matter.
		unsigned nov = evaluate_novelty(node, _wg_novelty_evaluators, 2, node.has_parent(), type, ptype);
		
		assert(node.w_g != Novelty::Unknown);
		if (node.w_g != Novelty::One) {
			node.w_g = (nov == 2) ? Novelty::Two : Novelty::GTTwo;
		}
		return nov;
	}
	
	template <typename NodeT>
	unsigned compute_node_complex_type(NodeT& node) {
		return compute_node_complex_type(node.unachieved_subgoals, node.get_relevant_atoms(*this).num_reached());
	}
	
	template <typename NodeT>
	unsigned evaluate_wgr1(NodeT& node) {
		unsigned type = compute_node_complex_type(node);
// 		bool has_parent = node.has_parent() && (node.parent->w_gr != Novelty::Unknown);
		bool has_parent = node.has_parent();
		unsigned ptype = has_parent ? compute_node_complex_type(*(node.parent)) : 0;
		unsigned nov = evaluate_novelty(node, _wgr_novelty_evaluators, 1, node.has_parent(), type, ptype);
		
		assert(node.w_gr == Novelty::Unknown);
		node.w_gr = (nov == 1) ? Novelty::One : Novelty::GTOne;
		return nov;
	}

	template <typename NodeT>
	unsigned evaluate_wgr2(NodeT& node) {
		assert(node._relevant_atoms.valid());
		unsigned type = compute_node_complex_type(node);
// 		bool has_parent = node.has_parent() && (node.parent->w_gr == Novelty::Two || node.parent->w_gr == Novelty::GTTwo); // i.e. the state has been evaluated on the novelty-two tables
		bool has_parent = node.has_parent();
		unsigned ptype = has_parent ? compute_node_complex_type(*(node.parent)) : 0;
		unsigned nov = evaluate_novelty(node, _wgr_novelty_evaluators, 2, node.has_parent(), type, ptype);
		
		assert(node.w_gr != Novelty::Unknown);
		if (node.w_gr != Novelty::One) {
			node.w_gr = (nov == 2) ? Novelty::Two : Novelty::GTTwo;
		}		
		return nov;
	}
	

	//! Return a newly-computed set of atoms which are relevant to reach the goal from the given state, with
	//! all those atoms marked as "unreached", and the rest as irrelevant.
	//! If 'log_stats' is true, the stats of this simulation will be logged in the '_stats' atribute.
	RelevantAtomSet compute_relevant_simulation(const State& state, unsigned& reachable) {
		reachable = 0;

		_iw_runner = std::unique_ptr<IWAlgorithm>(IWAlgorithm::build(_model, _featureset, _simulation_evaluator.clone(), _complete_simulation, _mark_negative_propositions));

		//BFWSStats stats;
		//StatsObserver<IWNodeT, BFWSStats> st_obs(stats, false);
		//iw->subscribe(st_obs);

		_iw_runner->run(state);

		RelevantAtomSet relevant = _iw_runner->retrieve_relevant_atoms(state, reachable);

		//LPT_INFO("cout", "IW Simulation: Node expansions: " << stats.expanded());
		//LPT_INFO("cout", "IW Simulation: Node generations: " << stats.generated());

		return relevant;
	}

	RelevantAtomSet compute_relevant(const State& state, bool log_stats) {
		unsigned reachable = 0, max_reachable = _model.num_subgoals();
		_unused(max_reachable);
		RelevantAtomSet relevant(nullptr);
		if (_aptk_rpg) {
			relevant = compute_relevant_aptk_hff(state);
		} else if (_use_simulation_nodes) {
			// Leave the relevant atom set empty
			compute_relevant_simulation(state, reachable);
		} else {
			relevant = compute_relevant_simulation(state, reachable);
		}

		LPT_EDEBUG("simulation-relevant", "Computing R(s) from state: " << std::endl << state << std::endl);
		LPT_EDEBUG("simulation-relevant", relevant.num_unreached() << " relevant atoms (" << reachable << "/" << max_reachable << " reachable subgoals): " << print::relevant_atomset(relevant) << std::endl << std::endl);

		if (log_stats) {
			_stats.set_initial_reachable_subgoals(reachable);
			_stats.set_initial_relevant_atoms(relevant.num_unreached());
		}
		_stats.reachable_subgoals(reachable);
		_stats.relevant_atoms(relevant.num_unreached());
		_stats.simulation();

		return relevant;
	}

	RelevantAtomSet compute_relevant_aptk_hff(const State& state) {
		assert(_aptk_rpg);
		const AtomIndex& atomidx = _problem.get_tuple_index();
		return _aptk_rpg->compute_r_ff(state, atomidx);
	}


	const std::unordered_set<IWNodePT>& get_last_simulation_nodes() const { return _iw_runner->get_relevant_nodes(); }

	//! This is a hackish way to obtain an integer index that uniquely identifies the tuple <#g, #r>
	unsigned compute_node_complex_type(unsigned unachieved, unsigned relaxed_achieved) {
		auto ind = _indexer(unachieved, relaxed_achieved);
#ifdef DEBUG
		// Let's make sure that either it's the first time we see this index, or, if was already there,
		// it corresponds to the same combination of <unachieved, relaxed_achieved>
		auto tuple = _indexer.relevant(unachieved, relaxed_achieved);
		auto __it =  __novelty_idx_values.find(ind);
		if (__it == __novelty_idx_values.end()) {
			__novelty_idx_values.insert(std::make_pair(ind, tuple));
		} else {
			assert(__it->second == tuple);
		}
#endif
		return ind;
	}



	template <typename NodeT>
	unsigned evaluate_novelty(const NodeT& node, NoveltyEvaluatorMapT& evaluator_map,  unsigned k, bool has_parent, unsigned type, unsigned parent_type) {
		auto it = evaluator_map.find(type);
		if (it == evaluator_map.end()) {
			auto inserted = evaluator_map.insert(std::make_pair(type, _search_evaluator.clone()));
			it = inserted.first;
		}
		FSBinaryNoveltyEvaluatorI* evaluator = it->second;

		if (has_parent && type == parent_type) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			return evaluator->evaluate(_featureset.evaluate(node.state), _featureset.evaluate(node.parent->state), k);
		}

		return evaluator->evaluate(_featureset.evaluate(node.state), k);


	}

	template <typename NodeT>
	void update_relevant_atoms(NodeT& node) {
		// Only for the root node _or_ whenever the number of unachieved nodes decreases
		// do we recompute the set of relevant atoms.
		State* marking_parent = nullptr;

		if (node.decreases_unachieved_subgoals()) {
			node._relevant_atoms = compute_relevant(node.state, !node.has_parent()); // Log only the stats of the seed state of the search.
		} else {
			// We copy the map of reached values from the parent node
			node._relevant_atoms = node.parent->get_relevant_atoms(*this); // This might trigger a recursive computation
		}

		// For the seed of the simulation, we want to mark all the initial atoms as reached.
		// But otherwise, we might want to mark as reached those atoms that change of value with respect to the parent.
// 		if (node.has_parent() && !node.decreases_unachieved_subgoals()) marking_parent = &(node.parent->state);
		if (node.has_parent()) marking_parent = &(node.parent->state);

		// In both cases, we update the set of relevant nodes with those that have been reached.
		node._relevant_atoms.mark(node.state, marking_parent, RelevantAtomSet::STATUS::REACHED, _mark_negative_propositions, true);
	}
	
	template <typename NodeT>
	void run_simulation(NodeT& node) {
		assert(_use_simulation_nodes);
		if (node._simulated) return;
		
		node._simulated = true;
		
		unsigned reachable = 0, max_reachable = _model.num_subgoals();
		_unused(max_reachable);
		// Leave the relevant atom set empty
		compute_relevant_simulation(node.state, reachable);

		LPT_EDEBUG("simulation-relevant", "Running simulation from state: " << std::endl << node.state << std::endl);
		LPT_EDEBUG("simulation-relevant", " " << reachable << "/" << max_reachable << " reachable subgoals" << std::endl << std::endl);

		if (!node.has_parent()) {
			_stats.set_initial_reachable_subgoals(reachable);
		}
		_stats.reachable_subgoals(reachable);
		_stats.simulation();
	}

	unsigned compute_unachieved(const State& state) {
		return _unsat_goal_atoms_heuristic.evaluate(state);
	}

protected:
#ifdef DEBUG
	// Just for sanity check purposes
	std::map<unsigned, std::tuple<unsigned,unsigned>> __novelty_idx_values;
#endif

	const StateModelT& _model;

	const Problem& _problem;

	const FeatureSetT& _featureset;

	// We keep a base evaluator to be cloned each time a new one is needed, so that there's no need
	// to perform all the feature selection, etc. anew.
	const FSBinaryNoveltyEvaluatorI& _search_evaluator;
	const FSBinaryNoveltyEvaluatorI& _simulation_evaluator;


	//! The novelty evaluators for the different #g values
	NoveltyEvaluatorMapT _wg_novelty_evaluators;

	//! The novelty evaluators for the different <#g, #r> values
	NoveltyEvaluatorMapT _wgr_novelty_evaluators;

	//! An UnsatisfiedGoalAtomsHeuristic to count the number of unsatisfied goals
	UnsatisfiedGoalAtomsHeuristic _unsat_goal_atoms_heuristic;

	NoveltyIndexerT _indexer;
	bool _mark_negative_propositions;
	bool _complete_simulation;

	BFWSStats& _stats;

	APTKFFHeuristicPT _aptk_rpg;
	
	bool _use_simulation_nodes;

	//! The last used iw-simulator, if any
	std::unique_ptr<IWAlgorithm> _iw_runner;
};


//! A specialized BFWS search schema with multiple queues to implement
//! effectively lazy novelty evaluation.
template <typename StateModelT>
class LazyBFWS {
public:
	using StateT = typename StateModelT::StateT;
	using ActionT = typename StateModelT::ActionType;
	using ActionIdT = typename ActionT::IdType;
	using NodeT = LazyBFWSNode<fs0::State, ActionT>;
	using PlanT =  std::vector<ActionIdT>;
	using NodePT = std::shared_ptr<NodeT>;
	using ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>;
	using HeuristicT = LazyBFWSHeuristic<StateModelT, SBFWSNoveltyIndexer>;
	using SimulationNodeT = typename HeuristicT::IWNodeT;
	using SimulationNodePT = typename HeuristicT::IWNodePT;

public:

	//! The only allowed constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the open list object to be used in the search
	//! (3) the closed list object to be used in the search
	LazyBFWS(const StateModelT& model, HeuristicT& heuristic, BFWSStats& stats, const Config& config, SBFWSConfig& conf) :
		_model(model),
		_solution(nullptr),
		_heuristic(heuristic),
		_stats(stats),
		_run_simulation_from_root(config.getOption<bool>("bfws.init_simulation", false)),
		_prune_wgr2_gt_2(config.getOption<bool>("bfws.prune", false)),
		_use_simulation_as_macros_only(conf.relevant_set_type==SBFWSConfig::RelevantSetType::Macro),
		_generated(0),
		_min_subgoals_to_reach(std::numeric_limits<unsigned>::max()),
		_novelty_levels(setup_novelty_levels(model))
	{
	}

	~LazyBFWS() = default;
	LazyBFWS(const LazyBFWS&) = delete;
	LazyBFWS(LazyBFWS&&) = default;
	LazyBFWS& operator=(const LazyBFWS&) = delete;
	LazyBFWS& operator=(LazyBFWS&&) = default;
	
	unsigned setup_novelty_levels(const StateModelT& model) const {
		const AtomIndex& atomidx = model.getTask().get_tuple_index();
		const unsigned num_subgoals = model.num_subgoals();
		const unsigned partition_size = num_subgoals * 10;   /// ???? What value expected for |R|??
		const unsigned num_positive_atoms = atomidx.size() / 2;
		
		float size_novelty2_tables = ( (float) pow(num_positive_atoms, 2) / 1024000.)  * (float) partition_size * 4;
		
		unsigned levels = (size_novelty2_tables > 2048) ? 2 : 3;
		
		LPT_INFO("cout", "Size of the novelty-two tables estimated at: " << size_novelty2_tables);
		LPT_INFO("cout", "Novelty levels of the search:  " << levels);
		
		return levels;
	}
	

	//! Convenience method
	bool solve_model(PlanT& solution) { return search(_model.init(), solution); }

	std::vector<NodePT> convert_simulation_nodes(const NodePT& root, const std::unordered_set<SimulationNodePT>& sim_nodes) {

		// Convert a set of simulation nodes into a set of search nodes
		// This is straight-forward except for setting up correctly the pointers to the parent node,
		// for which we need a second pass and the help of a couple of maps.
		std::unordered_map<SimulationNodePT, NodePT> simulation_to_search;
		std::unordered_map<NodePT, SimulationNodePT> search_to_simulation;

		std::vector<NodePT> search_nodes;
		search_nodes.reserve(sim_nodes.size());

		for (const SimulationNodePT& sim_node:sim_nodes) {
			NodePT search_node ;
			if (sim_node->parent == nullptr) { // We don't want to duplicate the root node
				search_node = root;
			} else {
				// We just update those attributes we're interested in
				search_node = std::make_shared<NodeT>(sim_node->state, _generated++); // TODO This is expensive, as it involves a full copy of the state, which could perhaps be moved.
				search_node->g = sim_node->g;
				search_node->action = sim_node->action;
				search_node->w_g = Novelty::One; // we enforce this by definition
				search_node->unachieved_subgoals = _heuristic.compute_unachieved(search_node->state);
			}

			simulation_to_search.insert(std::make_pair(sim_node, search_node));
			search_to_simulation.insert(std::make_pair(search_node, sim_node));
			search_nodes.push_back(search_node);

			//std::cout << "Simulation node " << sim_node << " corresponds to search node " << search_node << std::endl;
		}

		// Let n be any search node (note that the root node has been already discarded), and s be its corresponding simulation node
		// The parent of n is the search node that corresponds to parent(s)
		std::vector<NodePT> relevant;
		for (const NodePT& search_node:search_nodes) {
			if (search_node == root) continue; // We won't return the root node of the simulation, as it is already processed elsewhere

			auto it = search_to_simulation.find(search_node);
			assert(it != search_to_simulation.end());

			const auto& sim_node = it->second;
			assert(sim_node->parent != nullptr);
			auto it2 = simulation_to_search.find(sim_node->parent);
			//std::cout << "Simulation node parent: " << sim_node->parent << std::endl;
			assert(it2 != simulation_to_search.end());
			search_node->parent = it2->second;
			
			// ATM we only want to process the subgoal-satisfiyng end node of the path, and thus will pretend
			// pretend that intermediate nodes have already been processed
			// This works because we're using shared_ptrs!
			if (sim_node->satisfies_subgoal) {
				relevant.push_back(search_node);
			} else {
// 				std::cout << "Does NOT satisfy subgoal" << std::endl;
			}
		}

		return relevant;
	}


	bool search(const StateT& s, PlanT& plan) {
		NodePT root = std::make_shared<NodeT>(s, _generated++);
		create_node(root);
		assert(_q1.size()==1); // The root node must necessarily have novelty 1

// 		if (_run_simulation_from_root) {
// 			preprocess(root); // Preprocess the root node only
// 		}

		// The main search loop
		_solution = nullptr; // Make sure we start assuming no solution found

		for (bool remaining_nodes = true; !_solution && remaining_nodes;) {
			remaining_nodes = process_one_node();
		}

		return extract_plan(_solution, plan);
	}

protected:
	void dump_simulation_nodes(NodePT& node) {
		_heuristic.run_simulation(*node);
		const auto& simulation_nodes = _heuristic.get_last_simulation_nodes();
		auto search_nodes = convert_simulation_nodes(node, simulation_nodes);
		// std::cout << "Got " << simulation_nodes.size() << " simulation nodes, of which " << search_nodes.size() << " reused" << std::endl;
		for (const auto& n:search_nodes) {
			//create_node(n);
			_q1.insert(n);
			_stats.simulation_node_reused();
			// std::cout << "Simulation node reused: " << *n << std::endl;
		}
	}

	//! Process one node from some of the queues, according to their priorities
	bool process_one_node() {
		///// Q1 QUEUE /////
		// First process nodes with w_{#g}=1
		if (!_q1.empty()) {
			LPT_EDEBUG("multiqueue-search", "Picking node from Q1");
			NodePT node = _q1.next();
			process_node(node);
			_stats.wg1_node();
			return true;
		}


		///// QWGR1 QUEUE /////
		// Check whether there are nodes with w_{#g, #r} = 1
		if (!_use_simulation_as_macros_only && !_qwgr1.empty()) {
			LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g,#r} = 1");
			NodePT node = _qwgr1.next();

			// Compute wgr1 (this will compute #r lazily if necessary), and if novelty is one, expand the node.
			// Note that we _need_ to process the node through the wgr1 tables even if the node itself
			// has already been processed, for the sake of complying with the proper definition of novelty.
			unsigned nov = _heuristic.evaluate_wgr1(*node);
			if (!node->_processed && nov == 1) {
				_stats.wgr1_node();
				process_node(node);
			} else if (_novelty_levels == 2) {
				_qrest.insert(node);
			}

			// We might have processed one node but found no goal, let's start the loop again in case some node with higher priority was generated
			return true;
		}


		///// QWGR2 QUEUE /////
		// Check whether there are nodes with w_{#g, #r} = 2
		if (_novelty_levels == 3 && !_qwgr2.empty()) {
			LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g,#r} = 2");
			NodePT node = _qwgr2.next();

			unsigned nov;
			if (_use_simulation_as_macros_only) {
				if ( node->decreases_unachieved_subgoals()) {
					dump_simulation_nodes(node);
				}
				nov = _heuristic.evaluate_wg2(*node);
			} else {
				nov = _heuristic.evaluate_wgr2(*node);
			}

			// If the node has already been processed, no need to do anything else with it,
			// since we've already run it through all novelty tables.
			if (!node->_processed) {
				if (nov == 2) { // i.e. the node has exactly w_{#, #r} = 2
					_stats.wgr2_node();
					process_node(node);
				} else {
					if (!_prune_wgr2_gt_2) {
						_qrest.insert(node);
					}
				}
			}

			return true;
		}

		///// Q_REST QUEUE /////
		// Process the rest of the nodes, i.e. those with w_{#g, #r} > 1
		// We only extract one node and process it, as this will hopefully yield nodes with low novelty
		// that will thus have more priority than the rest of nodes in this queue.
		if (!_qrest.empty()) {
			LPT_EDEBUG("multiqueue-search", "Expanding one remaining node with w_{#g, #r} > 1");
			NodePT node = _qrest.next();
			_stats.wgr_gt2_node();
			process_node(node);
			return true;
		}

		return false;
	}


	//! When opening a node, we compute #g and evaluates whether the given node has <#g>-novelty 1 or not;
	//! if that is the case, we insert it into a special queue.
	void create_node(const NodePT& node) {
		node->unachieved_subgoals = _heuristic.compute_unachieved(node->state);
		
		if (node->unachieved_subgoals < _min_subgoals_to_reach) {
			_min_subgoals_to_reach = node->unachieved_subgoals;
			LPT_INFO("cout", "Min. # unreached subgoals: " << _min_subgoals_to_reach << "/" << _model.num_subgoals());
		}

		// Now insert the node into the appropriate queues
		_heuristic.evaluate_wg1(*node);
		if (node->w_g == Novelty::One) {
			_q1.insert(node);
		}

		_qwgr1.insert(node); // The node is surely pending evaluation in the w_{#g,#r}=1 tables
		
		if (_novelty_levels == 3) {
			_qwgr2.insert(node); // The node is surely pending evaluation in the w_{#g,#r}=2 tables
		}

		_stats.generation();
		if (node->decreases_unachieved_subgoals()) _stats.generation_g_decrease();
	}

	//! Process the node. Return true iff at least one node was created during the processing.
	void process_node(const NodePT& node) {

		//assert(!node->_processed); // Don't process a node twice!

// 		if (!node->from_simulation) {
			node->_processed = true; // Mark the node as processed
// 		}

		if (is_goal(node)) {
			LPT_INFO("cout", "Goal found");
			_solution = node;
			return;
		}

		_closed.put(node);
		expand_node(node);
	}

	// Return true iff at least one node was created
	unsigned expand_node(const NodePT& node) {
		LPT_DEBUG("cout", *node);
		_stats.expansion();
		if (node->decreases_unachieved_subgoals()) _stats.expansion_g_decrease();

		unsigned created = 0;

		for (const auto& action:_model.applicable_actions(node->state)) {
			StateT s_a = _model.next(node->state, action);
			NodePT successor = std::make_shared<NodeT>(std::move(s_a), action, node, _generated++);

			if (_closed.check(successor)) continue; // The node has already been closed
			if (is_open(successor)) continue; // The node is currently on (some) open list, so we ignore it

			create_node(successor);
			++created;
		}
		return created;
	}

	bool is_open(const NodePT& node) const {
		return _q1.contains(node) ||
		       _qwgr1.contains(node) ||
		       _qwgr2.contains(node) ||
		       _qrest.contains(node);
	}

	bool some_queue_nonempty() const {
		return !_q1.empty() ||
		       !_qwgr1.empty() ||
		       !_qwgr2.empty() ||
		       !_qrest.empty();
	}

	inline bool is_goal(const NodePT& node) const {
		return _model.goal(node->state);
	}

	//! Returns true iff there is an actual plan (i.e. because the given solution node is non-null)
	bool extract_plan(const NodePT& solution_node, PlanT& plan) const {
		if (!solution_node) return false;
		assert(plan.empty());

		NodePT node = solution_node;

		while (node->parent) {
			plan.push_back(node->action);
			node = node->parent;
		}

		std::reverse(plan.begin(), plan.end());
		return true;
	}

protected:
	// An open list sorted by #g
	using UnachievedSubgoalsComparerT = unachieved_subgoals_comparer<NodePT>;
	using UnachievedOpenList = lapkt::UpdatableOpenList<NodeT, NodePT, UnachievedSubgoalsComparerT>;
	
	//! An open list sorted by the numerical value of width, then #g
	using NoveltyComparerT = novelty_comparer<NodePT>;
	using StandardOpenList = lapkt::UpdatableOpenList<NodeT, NodePT, NoveltyComparerT>;
	
	using SearchableQueue = lapkt::SearchableQueue<NodeT>;


	//! The search model
	const StateModelT& _model;

	//! The solution node, if any. This will be set during the search process
	NodePT _solution;

	//! A list with all nodes that have novelty w_{#g}=1
	UnachievedOpenList _q1;

	//! A queue with those nodes that still need to be processed through the w_{#g, #r} = 1 novelty tables
	UnachievedOpenList _qwgr1;

	//! A queue with those nodes that still need to be processed through the w_{#g, #r} = 2 novelty tables
	UnachievedOpenList _qwgr2;

	//! A queue with those nodes that have been run through all relevant novelty tables
	//! and for which it has been proven that they have w_{#g, #r} > 2 and have not
	//! yet been processed.
	UnachievedOpenList _qrest;

	//! The closed list
	ClosedListT _closed;

	//! The heuristic object that will help us perform node evaluations
	HeuristicT& _heuristic;

	BFWSStats& _stats;

	//! Whether we want to run a simulation from the root node before starting the search
	bool _run_simulation_from_root;

	//! Whether we want to prune those nodes with novelty w_{#g, #r} > 2 or not
	bool _prune_wgr2_gt_2;
	
	bool _use_simulation_as_macros_only;

	//! The number of generated nodes so far
	unsigned long _generated;
	
	//! The minimum number of subgoals-to-reach that we have achieved at any moment of the search
	unsigned _min_subgoals_to_reach;
	
	//! How many novelty levels we want to use in the search.
	unsigned _novelty_levels;	
};





//! The main Simulated BFWS driver, sets everything up and runs the search.
//! The basic setup is:
//!
//! - Perform a (ATM naive) (novelty-) feature selection.
//!
//! - Create a "base" novelty evaluatorfor the search and a different one for the simulation.
//!   They might e.g. have different max_widths, or different features.
//!
//! - Instantiate the SBFWSHeuristic heuristic object, which will compute each node novelty,
//!   number of unachieved nodes #g and (if necessary) set of relevant atoms R.
//!
//! - Instantiate the (GBFS) search object, and start the search with it.
//!
template <typename StateModelT>
class LazyBFWSDriver : public drivers::Driver {
public:
	using StateT = typename StateModelT::StateT;
	using ActionT = typename StateModelT::ActionType;
	using EngineT = LazyBFWS<StateModelT>;
	using EnginePT = std::unique_ptr<EngineT>;
	using NodeT = LazyBFWSNode<StateT, ActionT>;
	using HeuristicT = typename EngineT::HeuristicT;
	using FeatureSetT = lapkt::novelty::StraightBinaryFeatureSetEvaluator<StateT>;
	using NoveltyEvaluatorPT = std::unique_ptr<FSBinaryNoveltyEvaluatorI>;
	

	//! Factory method
	EnginePT create(const Config& config, SBFWSConfig& conf, const NoveltyFeaturesConfiguration& feature_configuration, const StateModelT& model) {

		// Create here one instance to be copied around, so that no need to keep reanalysing which features are relevant
		_featureset = FeatureSetT();

		
		_search_evaluator = NoveltyEvaluatorPT(create_novelty_evaluator(model.getTask(), config, conf.search_width));
		_simulation_evaluator = NoveltyEvaluatorPT(create_novelty_evaluator(model.getTask(), config, conf.simulation_width));
		
		_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(conf, model, _featureset, *_search_evaluator, *_simulation_evaluator, _stats));

		auto engine = EnginePT(new EngineT(model, *_heuristic, _stats, config, conf));

// 		drivers::EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
// 		drivers::EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, _stats, _handlers);
// 		lapkt::events::subscribe(*engine, _handlers);

		return engine;
	}
	
	FSBinaryNoveltyEvaluatorI* create_novelty_evaluator(const Problem& problem, const Config& config, unsigned max_width) {
		
		if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
			const AtomIndex& index = problem.get_tuple_index();
			auto evaluator = FSAtomNoveltyEvaluator::create(index, true);
			if (evaluator) {
				LPT_INFO("cout", "Using a specialized FS Atom Novelty Evaluator");
				return evaluator;
			}
		}
		
		LPT_INFO("cout", "Using a Binary Novelty Evaluator");
		return new FSBinaryNoveltyEvaluator(max_width);
	}

	const BFWSStats& getStats() const { return _stats; }

	ExitCode search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) override;

protected:
	//!
	std::unique_ptr<HeuristicT> _heuristic;

	//!
	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;

	//!
	BFWSStats _stats;

	//! The feature set used for the novelty computations
	FeatureSetT _featureset;

	//! We keep a "base" novelty evaluator with the appropriate features, which (ATM)
	//! we will use for both search and heuristic simulation
	NoveltyEvaluatorPT _search_evaluator;
	NoveltyEvaluatorPT _simulation_evaluator;

	//! Helper
	ExitCode do_search(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time);
};


} } // namespaces
