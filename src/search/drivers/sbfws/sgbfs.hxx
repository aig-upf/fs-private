
#pragma once

#include <search/drivers/sbfws/iw_run.hxx>
#include <search/drivers/registry.hxx>
#include <search/drivers/setups.hxx>
#include <search/drivers/sbfws/base.hxx>
#include <heuristics/unsat_goal_atoms/unsat_goal_atoms.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>
#include <heuristics/novelty/features.hxx>
#include <lapkt/components/open_lists.hxx>

namespace fs0 { namespace bfws {


//! Prioritize nodes with lower number of _un_achieved subgoals
template <typename NodePT>
struct unachieved_subgoals_comparer { bool operator()(const NodePT& n1, const NodePT& n2) const { return n1->unachieved_subgoals > n2->unachieved_subgoals; } };



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

	//! The (cached) feature valuation corresponding to the state in this node
	lapkt::novelty::FeatureValuation feature_valuation;

	//! The novelty "type", i.e. the values wrt which the novelty is computed, e.g. <#g, #r>
	unsigned _type;

	//! The novelty of the state
	unsigned wg;

	//! The number of unachieved goals (#g)
	unsigned unachieved_subgoals;

	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
	RelevantAtomSet _relevant_atoms;
	
	bool _processed;

public:
	LazyBFWSNode() = delete;
	~LazyBFWSNode() = default;

	LazyBFWSNode(const LazyBFWSNode&) = delete;
	LazyBFWSNode(LazyBFWSNode&&) = delete;
	LazyBFWSNode& operator=(const LazyBFWSNode&) = delete;
	LazyBFWSNode& operator=(LazyBFWSNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	LazyBFWSNode(const StateT& s) : LazyBFWSNode(StateT(s), ActionT::invalid_action_id, nullptr) {}

	//! Constructor with move of the state (cheaper)
	LazyBFWSNode(StateT&& _state, action_t action_, ptr_t parent_) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0),
		wg(std::numeric_limits<unsigned>::max()),
		unachieved_subgoals(std::numeric_limits<unsigned>::max()),
		_relevant_atoms(nullptr),
		_processed(false)
	{}

	//! The novelty type (i.e. value wrt which the novelty is computed)
	unsigned type() const { return _type; }

	bool has_parent() const { return parent != nullptr; }

	bool operator==( const LazyBFWSNode<StateT, ActionT>& o ) const { return state == o.state; }

	bool dead_end() const { return false; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const LazyBFWSNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		return os << "{@ = " << this << ", s = " << state << ", g = " << g << ", wg=" << wg << ", #g=" << unachieved_subgoals << ", #r=" << _relevant_atoms.num_reached() << ", t=" << _type << ", parent = " << parent << "}";
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
	using FeatureSetT = lapkt::novelty::FeatureSet<State>;
	using NoveltyEvaluatorMapT = std::unordered_map<long, IWNoveltyEvaluator>;
	
	LazyBFWSHeuristic(const StateModelT& model, const FeatureSetT& features, const IWNoveltyEvaluator& search_evaluator, const IWNoveltyEvaluator& simulation_evaluator, BFWSStats& stats, bool mark_negative_propositions) :
		_model(model),
		_problem(model.getTask()),
		_featureset(features),
		_search_evaluator(search_evaluator),
		_simulation_evaluator(simulation_evaluator),
		_wg_novelty_evaluators(),
		_wgr_novelty_evaluators(),
		_unsat_goal_atoms_heuristic(_problem),
		_mark_negative_propositions(mark_negative_propositions),
		_stats(stats)
	{}

	~LazyBFWSHeuristic() = default;
	
	template <typename NodeT>
	unsigned evaluate_wg1(NodeT& node) {
		node._type = node.unachieved_subgoals;
		node.feature_valuation = _featureset.evaluate(node.state);
		return evaluate_novelty(node, _wg_novelty_evaluators, 1);
	}
	
	template <typename NodeT>
	unsigned evaluate_wgr1(NodeT& node) {
		node._type = compute_node_complex_type(node.unachieved_subgoals, node.get_relevant_atoms(*this).num_reached());
		node.feature_valuation = _featureset.evaluate(node.state);
		return evaluate_novelty(node, _wgr_novelty_evaluators, 1);
	}
	
	template <typename NodeT>
	unsigned evaluate_wgr2(NodeT& node) {
		// The type and feature valuation of the node are already cached therein.
		assert(node._relevant_atoms.valid());
		return evaluate_novelty(node, _wgr_novelty_evaluators, 2);
	}

	//! Return a newly-computed set of atoms which are relevant to reach the goal from the given state, with
	//! all those atoms marked as "unreached", and the rest as irrelevant.
	RelevantAtomSet compute_relevant(const State& state, bool log_stats) const {
		using ActionT = typename StateModelT::ActionType;
		using NodeT = IWRunNode<State, ActionT>;
		using IWAlgorithm = IWRun<NodeT, StateModelT>;
		
		if (_simulation_evaluator.max_novelty() == 0) { // No need to run anything
			return RelevantAtomSet(&(_problem.get_tuple_index()));
		}
		
		_stats.simulation();
		
		auto iw = std::unique_ptr<IWAlgorithm>(IWAlgorithm::build(_model, _featureset, _simulation_evaluator, _mark_negative_propositions));
		
		//BFWSStats stats;
		//StatsObserver<NodeT, BFWSStats> st_obs(stats, false);
		//iw->subscribe(st_obs);
		
		iw->run(state);
		unsigned reachable = 0;
		RelevantAtomSet relevant = iw->retrieve_relevant_atoms(state, reachable);
		
		//LPT_INFO("cout", "IW Simulation: Node expansions: " << stats.expanded());
		//LPT_INFO("cout", "IW Simulation: Node generations: " << stats.generated());

		if (log_stats) {
			_stats.set_initial_reachable_subgoals(reachable);
			_stats.set_initial_relevant_atoms(relevant.num_unreached());
		}

		return relevant;
	}

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
	unsigned evaluate_novelty(const NodeT& node, NoveltyEvaluatorMapT& evaluator_map, unsigned max_novelty = std::numeric_limits<unsigned>::max()) {
		auto it = evaluator_map.find(node._type);
		if (it == evaluator_map.end()) {
			auto inserted = evaluator_map.insert(std::make_pair(node._type, _search_evaluator));
			it = inserted.first;
		}
		IWNoveltyEvaluator& evaluator = it->second;
		return evaluator.evaluate(node, max_novelty);
	}

	template <typename NodeT>
	void update_relevant_atoms(NodeT& node) const {
		// Only for the root node _or_ whenever the number of unachieved nodes decreases
		// do we recompute the set of relevant atoms.
		if (node.decreases_unachieved_subgoals()) {
			node._relevant_atoms = compute_relevant(node.state, !node.has_parent());
		} else {
			// We copy the map of reached values from the parent node
			node._relevant_atoms = node.parent->get_relevant_atoms(*this);
		}

		// In both cases, we update the set of relevant nodes with those that have been reached.
		node._relevant_atoms.mark(node.state, nullptr, RelevantAtomSet::STATUS::REACHED, _mark_negative_propositions, true);
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
	const IWNoveltyEvaluator& _search_evaluator;
	const IWNoveltyEvaluator& _simulation_evaluator;

	
	//! The novelty evaluators for the different #g values
	NoveltyEvaluatorMapT _wg_novelty_evaluators;
	
	//! The novelty evaluators for the different <#g, #r> values
	NoveltyEvaluatorMapT _wgr_novelty_evaluators;

	//! An UnsatisfiedGoalAtomsHeuristic to count the number of unsatisfied goals
	UnsatisfiedGoalAtomsHeuristic _unsat_goal_atoms_heuristic;

	NoveltyIndexerT _indexer;
	bool _mark_negative_propositions;
	
	BFWSStats& _stats;
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
	
protected:
	enum class QPR_OUTPUT { // The result of processing any of the queues
		GOAL_FOUND, // The goal was found
		QUEUE_EMPTY, // Queue exhausted without finding a goal
		
	};
	
	enum class NPR_OUTPUT { // The result of processing a node
		GOAL_FOUND, // The goal was found while processing the node
		SOME_NODE_CREATED, // Some children node was actually generated while processing the node
		NO_NODE_CREATED // No children node was actually generated while processing the node
	};
	
public:

	//! The only allowed constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the open list object to be used in the search
	//! (3) the closed list object to be used in the search
	LazyBFWS(const StateModelT& model, HeuristicT& heuristic, BFWSStats& stats) :
		_model(model),
		_solution(nullptr),
		_heuristic(heuristic),
		_stats(stats)
	{
	}

	~LazyBFWS() = default;
	LazyBFWS(const LazyBFWS&) = delete;
	LazyBFWS(LazyBFWS&&) = default;
	LazyBFWS& operator=(const LazyBFWS&) = delete;
	LazyBFWS& operator=(LazyBFWS&&) = default;
	
	//! Convenience method
	bool solve_model(PlanT& solution) { return search(_model.init(), solution); }
	
	bool search(const StateT& s, PlanT& plan) {
		NodePT root = std::make_shared<NodeT>(s);
		
		create_node(root);
		assert(_q1.size()==1); // The root node must necessarily have novelty 1
		
		// The main search loop
		_solution = nullptr; // Make sure we start assuming no solution found
		
		for (bool remaining_nodes = true; !_solution && remaining_nodes;) {
			remaining_nodes = process_one_node();
		}
		
		/*
		while (!_solution && some_queue_nonempty()) {
			
			///// Q1 QUEUE /////
			// First process nodes with w_{#g}=1
			if (!_q1.empty()) {
				LPT_EDEBUG("multiqueue-search", "Picking node from Q1");
				NodePT node = _q1.next();
				process_node(node);
				continue; // Start the loop again
			}
			
			
			///// QWGR1 QUEUE /////
			// Check whether there are nodes with w_{#g, #r} = 1
			if (!_qwgr1.empty()) {
				LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g,#r} = 1");
				NodePT node = _qwgr1.next();
				
				// Compute wgr1 (this will compute #r lazily if necessary), and if novelty is one, expand the node.
				// Note that we _need_ to process the node through the wgr1 tables even if the node itself
				// has already been processed, for the sake of complying with the proper definition of novelty.
				unsigned nov = _heuristic.evaluate_wgr1(*node);
				if (!node->_processed && nov == 1) {
					if (process_node(node) == NPR_OUTPUT::GOAL_FOUND) break;
				}
				continue; // We might have processed one node but found no goal, let's start the loop again in case some node with higher priority was generated
			}
			
		
			///// QWGR2 QUEUE /////
			// Check whether there are nodes with w_{#g, #r} = 2
			if (!_qwgr2.empty()) {
				LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g,#r} = 2");
				NodePT node = _qwgr2.next();
				
				unsigned nov = _heuristic.evaluate_wgr2(*node);
				
				// If the node has already been processed, no need to do anything else with it,
				// since we've already run it through all novelty tables.
				if (!node->_processed) {
					if (nov == 2) { // i.e. the node has exactly w_{#, #r} = 2
						if (process_node(node) == NPR_OUTPUT::GOAL_FOUND) break;
					} else {
						_qrest.insert(node);
					}
				}
				
				continue;
			}
		
			///// Q_REST QUEUE /////
			// Process the rest of the nodes, i.e. those with w_{#g, #r} > 1
			// We only extract one node and process it, as this will hopefully yield nodes with low novelty
			// that will thus have more priority than the rest of nodes in this queue.
			if (!_qrest.empty()) {
				LPT_EDEBUG("multiqueue-search", "Expanding one remaining node with w_{#g, #r} > 1");
				NodePT node = _qrest.next();
				if (process_node(node) == NPR_OUTPUT::GOAL_FOUND) break;
			}
		}
		*/

		
		return extract_plan(_solution, plan);
	}
	
protected:
	
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
		if (!_qwgr1.empty()) {
			LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g,#r} = 1");
			NodePT node = _qwgr1.next();
			
			// Compute wgr1 (this will compute #r lazily if necessary), and if novelty is one, expand the node.
			// Note that we _need_ to process the node through the wgr1 tables even if the node itself
			// has already been processed, for the sake of complying with the proper definition of novelty.
			unsigned nov = _heuristic.evaluate_wgr1(*node);
			if (!node->_processed && nov == 1) {
				_stats.wgr1_node();
				process_node(node);
			}
			
			// We might have processed one node but found no goal, let's start the loop again in case some node with higher priority was generated
			return true;
		}
		
	
		///// QWGR2 QUEUE /////
		// Check whether there are nodes with w_{#g, #r} = 2
		if (!_qwgr2.empty()) {
			LPT_EDEBUG("multiqueue-search", "Checking for open nodes with w_{#g,#r} = 2");
			NodePT node = _qwgr2.next();
			
			unsigned nov = _heuristic.evaluate_wgr2(*node);
			
			// If the node has already been processed, no need to do anything else with it,
			// since we've already run it through all novelty tables.
			if (!node->_processed) {
				if (nov == 2) { // i.e. the node has exactly w_{#, #r} = 2
					_stats.wgr2_node();
					process_node(node);
				} else {
					_qrest.insert(node);
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
	
	
	/*
	//! Process _all_ nodes in Q1. Return true iff at least one new node is created during the process.
	QPR_OUTPUT process_q1_nodes() {
		while (!_q1.empty()) {
			NodePT node = _q1.next();
			NPR_OUTPUT output = process_node(node);
			if (output == NPR_OUTPUT::GOAL_FOUND) return QPR_OUTPUT::GOAL_FOUND;
			// Otherwise keep processing nodes from the queue
		}
		
		return QPR_OUTPUT::QUEUE_EMPTY;
	}
	
	
	//! Process one node in Qwrg1. Return true iff at least one new node is created during the process.
	QPR_OUTPUT process_qwgr1_nodes() {
		if (!_qwgr1.empty()) {
			NodePT node = _qwgr1.next();
			
			// Compute wgr1 (this will compute #r lazily if necessary), and if novelty is one, expand the node.
			// Note that we _need_ to process the node through the wgr1 tables even if the node itself
			// has already been processed, for the sake of complying with the proper definition of novelty.
			unsigned nov = _heuristic.evaluate_wgr1(*node);
			if (!node->_processed && nov == 1) {
				return process_node(node);
			}
		}
		
		return false;
	}
	
	//! Process nodes in Qwrg2. Return true iff at least one new node is created during the process.
	bool process_qwgr2_nodes() {
		if (_qwgr2.empty()) {
			NodePT node = _qwgr2.next();
			
			unsigned nov = _heuristic.evaluate_wgr2(*node);
			
			// If the node has already been processed, no need to do anything else with it,
			// since we've already run it through all novelty tables.
			if (!node->_processed) {
				if (nov == 2) { // i.e. the node has exactly w_{#, #r} = 2
					return process_node(node);
				} else {
					_qrest.insert(node);
				}
			}
		}
		return false;
	}
	
	//! Process nodes in Q_rest. Return true iff at least one new node is created during the process.
	bool process_qrest() {
		// We only extract one node and process it, as this will hopefully yield nodes with low novelty
		// that will thus have more priority than the rest of nodes in this queue.
		if (_qrest.empty()) return false;
		NodePT node = _qrest.next();
		return process_node(node);
	}
	*/

	//! When opening a node, we compute #g and evaluates whether the given node has <#g>-novelty 1 or not;
	//! if that is the case, we insert it into a special queue.
	void create_node(const NodePT& node) {
		node->unachieved_subgoals = _heuristic.compute_unachieved(node->state);
		
		// Now insert the node into the appropriate queues
		node->wg = _heuristic.evaluate_wg1(*node);
		if (node->wg == 1) {
			_q1.insert(node);
		}
		
		_qwgr1.insert(node); // The node is surely pending evaluation in the w_{#g,#r}=1 tables
		_qwgr2.insert(node); // The node is surely pending evaluation in the w_{#g,#r}=2 tables
		
		_stats.generation();
		if (node->decreases_unachieved_subgoals()) _stats.generation_g_decrease();
	}
	
	//! Process the node. Return true iff at least one node was created during the processing.
	NPR_OUTPUT process_node(const NodePT& node) {
		assert(!node->_processed); // Don't process a node twice!
		
		node->_processed = true; // Mark the node as processed
		
		if (is_goal(node)) {
			LPT_INFO("cout", "Goal found");
			_solution = node;
			return NPR_OUTPUT::GOAL_FOUND;
		}
		
		_closed.put(node);
		unsigned num_created = expand_node(node);
		return num_created > 0 ? NPR_OUTPUT::SOME_NODE_CREATED : NPR_OUTPUT::NO_NODE_CREATED;
		
	}
	
	// Return true iff at least one node was created
	unsigned expand_node(const NodePT& node) {
		LPT_DEBUG("cout", *node);
		_stats.expansion();
		if (node->decreases_unachieved_subgoals()) _stats.expansion_g_decrease();
		
		unsigned created = 0;
		
		for (const auto& action:_model.applicable_actions(node->state)) {
			StateT s_a = _model.next(node->state, action);
			NodePT successor = std::make_shared<NodeT>(std::move(s_a), action, node);
			
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
	using UnachievedSubgoalsComparerT = unachieved_subgoals_comparer<NodePT>;
	using WG1OpenList = lapkt::UpdatableOpenList<NodeT, NodePT, UnachievedSubgoalsComparerT>;
	using SearchableQueue = lapkt::SearchableQueue<NodeT>;


	//! The search model
	const StateModelT& _model;
	
	//! The solution node, if any. This will be set during the search process
	NodePT _solution;
	
	//! A list with all nodes that have novelty w_{#g}=1
	WG1OpenList _q1;
	
	//! A queue with those nodes that still need to be processed through the w_{#g, #r} = 1 novelty tables
	SearchableQueue _qwgr1;
	
	//! A queue with those nodes that still need to be processed through the w_{#g, #r} = 2 novelty tables
	SearchableQueue _qwgr2;
	
	//! A queue with those nodes that have been run through all relevant novelty tables 
	//! and for which it has been proven that they have w_{#g, #r} > 2 and have not 
	//! yet been processed.
	SearchableQueue _qrest;

	//! The closed list
	ClosedListT _closed;
	
	//! The heuristic object that will help us perform node evaluations
	HeuristicT& _heuristic;
	
	BFWSStats& _stats;
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
	using FeatureSetT = lapkt::novelty::FeatureSet<StateT>;

	//! Factory method
	EnginePT create(const Config& config, SBFWSConfig& conf, const NoveltyFeaturesConfiguration& feature_configuration, const StateModelT& model) {

		// Create here one instance to be copied around, so that no need to keep reanalysing which features are relevant
		_featureset = selectFeatures(feature_configuration);

		_search_evaluator = std::unique_ptr<IWNoveltyEvaluator>(new IWNoveltyEvaluator(conf.search_width));
		_simulation_evaluator = std::unique_ptr<IWNoveltyEvaluator>(new IWNoveltyEvaluator(conf.simulation_width));

		_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(model, _featureset, *_search_evaluator, *_simulation_evaluator, _stats, conf.mark_negative_propositions));

		auto engine = EnginePT(new EngineT(model, *_heuristic, _stats));

// 		drivers::EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
// 		drivers::EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, _stats, _handlers);
// 		lapkt::events::subscribe(*engine, _handlers);

		return engine;
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
	std::unique_ptr<IWNoveltyEvaluator> _search_evaluator;
	std::unique_ptr<IWNoveltyEvaluator> _simulation_evaluator;

	// ATM we don't perform any particular feature selection
	FeatureSetT selectFeatures(const NoveltyFeaturesConfiguration& feature_configuration) {
		const ProblemInfo& info = ProblemInfo::getInstance();

		FeatureSetT features;

		// Add all state variables
		for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
			features.add(std::unique_ptr<lapkt::novelty::NoveltyFeature<State>>(new StateVariableFeature(var)));
		}

		LPT_INFO("cout", "Number of features from which state novelty will be computed: " << features.size());
		return features;
	}

	//! Helper
	ExitCode do_search(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time);
};


} } // namespaces
