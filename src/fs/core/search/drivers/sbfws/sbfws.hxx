
#pragma once

#include <fs/core/search/drivers/sbfws/iw_run.hxx>
#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/heuristics/unsat_goal_atoms.hxx>
#include <fs/core/heuristics/l0.hxx>
//#include <fs/hybrid/heuristics/l2_norm.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>

#include <fs/core/search/drivers/sbfws/stats.hxx>
#include <fs/core/search/drivers/sbfws/relevant_atoms.hxx>
#include <fs/core/constraints/gecode/handlers/monotonicity_csp.hxx>


namespace fs0 { namespace bfws {

using Novelty = lapkt::novelty::Novelty;

//! Prioritize nodes with lower number of _un_achieved subgoals. Break ties with g.
template <typename NodePT>
struct unachieved_subgoals_comparer {
	bool operator()(const NodePT& n1, const NodePT& n2) const {
		if (n1->unachieved_subgoals > n2->unachieved_subgoals) return true;
		if (n1->unachieved_subgoals < n2->unachieved_subgoals) return false;
		if (n1->g > n2->g) return true;
		if (n1->g < n2->g) return false;
		if (n1->w_g == Novelty::One && n2->w_g != Novelty::One) return false;
// 		if (n1->w_gr == Novelty::One && n2->w_gr != Novelty::One) return false;
		return n1->_gen_order > n2->_gen_order;
	}
};

// ! Comparer taking into account #g and novelty
template <typename NodePT>
struct novelty_comparer {
	bool operator()(const NodePT& n1, const NodePT& n2) const {
		if (n1->w_g_r > n2->w_g_r) return true;
		if (n1->w_g_r < n2->w_g_r) return false;
		if (n1->unachieved_subgoals > n2->unachieved_subgoals) return true;
		if (n1->unachieved_subgoals < n2->unachieved_subgoals) return false;
		if (n1->g > n2->g) return true;
		if (n1->g < n2->g) return false;
		return n1->_gen_order > n2->_gen_order;
	}
};


//! The node type we'll use for the Simulated BFWS search, parametrized by type of state and action action
template <typename StateT, typename ActionT>
class SBFWSNode {
public:
	using ptr_t = std::shared_ptr<SBFWSNode<StateT, ActionT>>;
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
	uint32_t unachieved_subgoals;

	//! Whether the node has been processed
	bool _processed;

	//! The generation order, uniquely identifies the node
	//! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
	uint32_t _gen_order;

	//! The novelty w_{#g} of the state
	Novelty w_g;

	//! The novelty w_{#g,#r} of the state
	Novelty w_gr;

	//! A reference atomset helper wrt which the sets R of descendent nodes with same #g are computed
	//! Use a raw pointer to optimize performance, as the number of generated nodes will typically be huge
	AtomsetHelper* _helper;

	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
	//! Use a raw pointer to optimize performance, as the number of generated nodes will typically be huge
	RelevantAtomSet* _relevant_atoms;

	//! #r
	unsigned		_hash_r;

	//! The indexes of the variables whose atoms form the set 1(s), which contains all atoms in 1(parent(s)) not deleted by the action that led to s, plus those
	//! atoms in s with novelty 1.
// 	std::vector<unsigned> _nov1atom_idxs;

    //! The sets D^G_X of goal-reachable domains for every state variable X
    DomainTracker _domains;

	//! Constructor with full copying of the state (expensive)
	SBFWSNode(const StateT& s, unsigned long gen_order) : SBFWSNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	SBFWSNode(StateT&& _state, action_t action_, ptr_t parent_, uint32_t gen_order) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0),
		unachieved_subgoals(std::numeric_limits<unsigned>::max()),
		_processed(false),
		_gen_order(gen_order),
		w_g(Novelty::Unknown),
		w_gr(Novelty::Unknown),
		_helper(nullptr),
		_relevant_atoms(nullptr),
		_hash_r(0)
// 		_nov1atom_idxs()
	{
		assert(_gen_order > 0); // Very silly way to detect overflow, in case we ever generate > 4 billion nodes :-)
	}

	~SBFWSNode() { delete _helper; delete _relevant_atoms; }
	SBFWSNode(const SBFWSNode&) = delete;
	SBFWSNode(SBFWSNode&&) = delete;
	SBFWSNode& operator=(const SBFWSNode&) = delete;
	SBFWSNode& operator=(SBFWSNode&&) = delete;


	bool has_parent() const { return parent != nullptr; }

	bool operator==( const SBFWSNode<StateT, ActionT>& o ) const { return state == o.state; }

	bool dead_end() const { return false; }

	std::size_t hash() const { return state.hash(); }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const SBFWSNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
// 		const Problem& problem = Problem::getInstance();
		std::string reached = "?";
		if (_relevant_atoms) {
			reached = std::to_string(_relevant_atoms->num_reached()) + " / " + std::to_string(_relevant_atoms->getHelper()._num_relevant);
		} else {
			reached = std::to_string(_hash_r);
		}
		os << "#" << _gen_order << " (" << this << "), " << state;
		os << ", g = " << g << ", w_g" << w_g <<  ", w_gr" << w_gr << ", #g=" << unachieved_subgoals << ", #r=" << reached;
		os << ", parent = " << (parent ? "#" + std::to_string(parent->_gen_order) : "None");
		os << ", decr(#g)= " << this->decreases_unachieved_subgoals();
// 		if (action != ActionT::invalid_action_id) os << ", a = " << *problem.getGroundActions()[action];
		if (action != ActionT::invalid_action_id) os << ", a = " << action;
		else os << ", a = None";
		return os << "}";
	}

	bool decreases_unachieved_subgoals() const {
		return (!has_parent() || unachieved_subgoals < parent->unachieved_subgoals);
	}
};



template <typename StateModelT,
          typename NoveltyIndexerT,
          typename FeatureSetT,
          typename NoveltyEvaluatorT,
		  typename NodeT,
          template <class N, class S, class NE, class FS> class SimulatorT,
          template <class S, class A> class SimNodeT 
>
class SBFWSHeuristic {
public:
	using NoveltyEvaluatorMapT = std::unordered_map<long, NoveltyEvaluatorT*>;
	using ActionT = typename StateModelT::ActionType;
	using IWNodeT =  SimNodeT<State, ActionT>;
	using SimulationT =  SimulatorT<IWNodeT, StateModelT, NoveltyEvaluatorT, FeatureSetT>;
	

	// Novelty evaluator pointer type
	using NoveltyEvaluatorPT = std::unique_ptr<NoveltyEvaluatorT>;
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;


protected:
	const StateModelT& _model;

	const Problem& _problem;

	const FeatureSetT& _featureset;

	const NoveltyFactory<FeatureValueT> _search_novelty_factory;

	//! The novelty evaluators for the different #g values.
	//! The i-th position of the vector will actually contain the evaluator for novelty i+1
	std::vector<NoveltyEvaluatorMapT> _wg_novelty_evaluators;

	//! The novelty evaluators for the different <#g, #r> values
	//! The i-th position of the vector will actually contain the evaluator for novelty i+1
	std::vector<NoveltyEvaluatorMapT> _wgr_novelty_evaluators;

	//! A counter to count the number of unsatisfied goals
	UnsatisfiedGoalAtomsCounter _unsat_goal_atoms_heuristic;

	NoveltyIndexerT _indexer;
	bool _mark_negative_propositions;

	BFWSStats& _stats;

	SBFWSConfig _sbfwsconfig;
	
	std::unique_ptr<RelevantAtomsCounterI<NodeT>> _r_counter;


public:
	SBFWSHeuristic(const SBFWSConfig& config, const StateModelT& model, const FeatureSetT& features, BFWSStats& stats) :
		_model(model),
		_problem(model.getTask()),
		_featureset(features),
		_search_novelty_factory(_problem, config.evaluator_t, _featureset.uses_extra_features(), config.search_width),
		_wg_novelty_evaluators(3), // We'll only care about novelties 1 and, at most, 2.
		_wgr_novelty_evaluators(3), // We'll only care about novelties 1 and, at most, 2.
		_unsat_goal_atoms_heuristic(_problem),
		_mark_negative_propositions(config.mark_negative_propositions),
        _stats(stats),
        _sbfwsconfig(config),
        _r_counter(RelevantAtomsCounterFactory::build<StateModelT, NodeT, SimulationT, NoveltyEvaluatorT, FeatureSetT>(model, config, features))
    {
    }

	~SBFWSHeuristic() {
		for (auto& elem:_wg_novelty_evaluators) for (auto& p:elem) delete p.second;
		for (auto& elem:_wgr_novelty_evaluators) for (auto& p:elem) delete p.second;
	};


	unsigned evaluate_wg1(NodeT& node) {
		unsigned type = node.unachieved_subgoals;
		unsigned ptype = node.has_parent() ? node.parent->unachieved_subgoals : 0; // If the node has no parent, this value doesn't matter.
		unsigned nov = evaluate_novelty(node, _wg_novelty_evaluators, 1, type, ptype);
		assert(node.w_g == Novelty::Unknown);
		node.w_g = (nov == 1) ? Novelty::One : Novelty::GTOne;
		return nov;
	}

	unsigned evaluate_wg2(NodeT& node) {
		unsigned type = node.unachieved_subgoals;
		unsigned ptype = node.has_parent() ? node.parent->unachieved_subgoals : 0; // If the node has no parent, this value doesn't matter.
		unsigned nov = evaluate_novelty(node, _wg_novelty_evaluators, 2, type, ptype);

		assert(node.w_g != Novelty::Unknown);
		if (node.w_g != Novelty::One) {
			node.w_g = (nov == 2) ? Novelty::Two : Novelty::GTTwo;
		}
		return nov;
	}

	unsigned get_hash_r(NodeT& node) {
		return _r_counter->count(node, _stats);
	}

	unsigned compute_node_complex_type(NodeT& node) {
// 		LPT_INFO("types", "Type=" << compute_node_complex_type(node.unachieved_subgoals, get_hash_r(node)) << " for node: " << std::endl << node)
// 		LPT_INFO("hash_r", "#r=" << get_hash_r(node) << " for node: " << std::endl << node)
		return compute_node_complex_type(node.unachieved_subgoals, get_hash_r(node));
	}

	unsigned evaluate_wgr1(NodeT& node) {

		// A temporary hack: if we want no R computation at all, then return INF novelty w_{#g,#r} so that nodes on QWRG1 are ignored.
		// This poses a small overhead, but it is only temporary.
		if (_sbfwsconfig.relevant_set_type == SBFWSConfig::RelevantSetType::None) {
			node.w_gr = Novelty::GTOne;
			return std::numeric_limits<unsigned>::max();
		}

		unsigned type = compute_node_complex_type(node);
		unsigned ptype = node.has_parent() ? compute_node_complex_type(*(node.parent)) : 0;
		unsigned nov = evaluate_novelty(node, _wgr_novelty_evaluators, 1, type, ptype);

		assert(node.w_gr == Novelty::Unknown);
		node.w_gr = (nov == 1) ? Novelty::One : Novelty::GTOne;
		return nov;
	}

	unsigned evaluate_wgr2(NodeT& node) {
		unsigned type = compute_node_complex_type(node);
		unsigned ptype = node.has_parent() ? compute_node_complex_type(*(node.parent)) : 0;
		unsigned nov = evaluate_novelty(node, _wgr_novelty_evaluators, 2, type, ptype);

		assert(node.w_gr != Novelty::Unknown);
		if (node.w_gr != Novelty::One) {
			node.w_gr = (nov == 2) ? Novelty::Two : Novelty::GTTwo;
		}
		return nov;
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


	NoveltyEvaluatorT* fetch_evaluator(NoveltyEvaluatorMapT& evaluator_map,  unsigned k, unsigned type) {
		auto it = evaluator_map.find(type);
		if (it == evaluator_map.end()) {
			auto inserted = evaluator_map.insert(std::make_pair(type, _search_novelty_factory.create_evaluator(k)));
			_stats.search_table_created(k);
			it = inserted.first;
		}
		return it->second;
	}

	unsigned evaluate_novelty(const NodeT& node, std::vector<NoveltyEvaluatorMapT>& evaluator_map,  unsigned k, unsigned type, unsigned parent_type) {
		NoveltyEvaluatorT* evaluator = fetch_evaluator(evaluator_map[k], k, type);

		if (node.has_parent() && type == parent_type) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			return evaluator->evaluate(_featureset.evaluate(node.state), _featureset.evaluate(node.parent->state), k);
		}

		return evaluator->evaluate(_featureset.evaluate(node.state), k);
	}

	unsigned compute_unachieved(const State& state) {
		return _unsat_goal_atoms_heuristic.evaluate(state);
	}

protected:
#ifdef DEBUG
	// Just for sanity check purposes
	std::map<unsigned, std::tuple<unsigned,unsigned>> __novelty_idx_values;
#endif
};


//! A specialized BFWS search schema with multiple queues to implement
//! effectively lazy novelty evaluation.
template <typename StateModelT,
          typename FeatureSetT,
          typename NoveltyEvaluatorT,          
          template <class N, class S, class NE, class FS> class SimulatorT,
          template <class S, class A> class SimNodeT>
class SBFWS {
public:
	using StateT = typename StateModelT::StateT;
	using ActionT = typename StateModelT::ActionType;
	using ActionIdT = typename ActionT::IdType;
	using NodeT = SBFWSNode<fs0::State, ActionT>;
	using PlanT =  std::vector<ActionIdT>;
	using NodePT = std::shared_ptr<NodeT>;
	using ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>;
	using HeuristicT = SBFWSHeuristic<StateModelT, SBFWSNoveltyIndexer, FeatureSetT, NoveltyEvaluatorT, NodeT, SimulatorT, SimNodeT>;
	using SimulationNodeT = typename HeuristicT::IWNodeT;


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
	//! Best node found
	NodePT _best_found;

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

	//! The novelty feature evaluator.
	//! We hold the object here so that we can reuse the same featureset for search and simulations
	FeatureSetT _featureset;

	//! The heuristic object that will help us perform node evaluations
	HeuristicT _heuristic;

	BFWSStats& _stats;

	//! Whether we want to prune those nodes with novelty w_{#g, #r} > 2 or not
	bool _pruning;

	//! The number of generated nodes so far
	uint32_t _generated;

	//! The minimum number of subgoals-to-reach that we have achieved at any moment of the search
	unsigned _min_subgoals_to_reach;

	//! How many novelty levels we want to use in the search.
	unsigned _novelty_levels;

	std::unique_ptr<gecode::MonotonicityCSP> _monotonicity_csp_manager;

public:

	//!
	SBFWS(const StateModelT& model,
          FeatureSetT&& featureset,
          BFWSStats& stats,
          SBFWSConfig& config) :

		_model(model),
		_solution(nullptr),
        _best_found(nullptr),
		_featureset(std::move(featureset)),
		_heuristic(config, model, _featureset, stats),
		_stats(stats),
		_pruning(config._global_config.getOption<bool>("bfws.prune", false)),
		_generated(0),
		_min_subgoals_to_reach(std::numeric_limits<unsigned>::max()),
		_novelty_levels(setup_novelty_levels(model, config._global_config)),
        _monotonicity_csp_manager(gecode::build_monotonicity_csp(_model.getTask(), config._global_config))
	{
	}

	~SBFWS() = default;
	SBFWS(const SBFWS&) = delete;
	SBFWS(SBFWS&&) = default;
	SBFWS& operator=(const SBFWS&) = delete;
	SBFWS& operator=(SBFWS&&) = default;

	unsigned setup_novelty_levels(const StateModelT& model, const Config& global_config) const {
		const AtomIndex& atomidx = model.getTask().get_tuple_index();

		// Allow the user to override the automatic configuration of the levels of novelty
		int user_option = global_config.getOption<int>("novelty_levels", -1);
		if (user_option != -1) {
			if (user_option != 2 && user_option != 3) {
				throw std::runtime_error("Unsupported novelty levels: " + std::to_string(user_option));
			}

			LPT_INFO("search", "(User-specified) Novelty levels of the search:  " << user_option);
			return user_option;
		}

		const unsigned num_subgoals = model.num_subgoals();
		unsigned expected_R_size = 10; // TODO ???? What value expected for |R|??
		const unsigned num_atoms = atomidx.size();

		float size_novelty2_table = ((float) num_atoms*(num_atoms-1)+num_atoms) / (1024*1024*8.);
		float size_novelty2_tables = num_subgoals * expected_R_size * size_novelty2_table;

		unsigned levels = (size_novelty2_tables > 2048) ? 2 : 3;

		LPT_INFO("search", "Size of a single specialized novelty-2 table estimated at (MB): " << size_novelty2_table);
		LPT_INFO("search", "Expected overall size of all novelty-two tables (MB): " << size_novelty2_tables);
		LPT_INFO("search", "Novelty levels of the search:  " << levels);

		return levels;
	}


	//! Convenience method
	bool solve_model(PlanT& solution) { return search(_model.init(), solution); }

	bool search(const StateT& s, PlanT& plan) {
		NodePT root = std::make_shared<NodeT>(s, ++_generated);

        if (_monotonicity_csp_manager) {
            root->_domains = _monotonicity_csp_manager->create_root(s);
            if (root->_domains.is_null()) {
                LPT_INFO("cout", "Root node detected as inconsistent for monotonicity reasons");
                _stats.monot_pruned();
                return false;
            }
        }

        create_node(root);
		assert(_q1.size()==1); // The root node must necessarily have novelty 1


		// Force one simulation from the root node and abort the search
//        _heuristic.get_hash_r(*root);
//  		_heuristic.compute_R(*root);
// 		return false;


		// The main search loop
		_solution = nullptr; // Make sure we start assuming no solution found

		for (bool remaining_nodes = true; !_solution && remaining_nodes;) {
			remaining_nodes = process_one_node();
		}
//		if ( _solution == nullptr )
//			return extract_plan(_best_found, plan);

		return extract_plan(_solution, plan);
	}

protected:


	//! Process one node from some of the queues, according to their priorities
	//! Returns true if some action has been performed, false if all queues were empty
	bool process_one_node() {
		///// Q1 QUEUE /////
		// First process nodes with w_{#g}=1
		if (!_q1.empty()) {
			NodePT node = _q1.next();
			process_node(node);
			_stats.wg1_node();
			return true;
		}

		///// QWGR1 QUEUE /////
		// Check whether there are nodes with w_{#g, #r} = 1
		if (!_qwgr1.empty()) {
			NodePT node = _qwgr1.next();

			// Compute wgr1 (this will compute #r lazily if necessary), and if novelty is one, expand the node.
			// Note that we _need_ to process the node through the wgr1 tables even if the node itself
			// has already been processed, for the sake of complying with the proper definition of novelty.
			unsigned nov = _heuristic.evaluate_wgr1(*node);

			if (!node->_processed) {
				if (nov == 1) {
					_stats.wgr1_node();
					process_node(node);
				} else {
					handle_unprocessed_node(node, (_novelty_levels == 2));
				}
			}

			// We might have processed one node but found no goal, let's start the loop again in case some node with higher priority was generated
			return true;
		}

		///// QWGR2 QUEUE /////
		// Check whether there are nodes with w_{#g, #r} = 2
		if (_novelty_levels == 3 && !_qwgr2.empty()) {
			NodePT node = _qwgr2.next();

			// unsigned nov = _heuristic.evaluate_wg2(*node);
			unsigned nov = _heuristic.evaluate_wgr2(*node);

			// If the node has already been processed, no need to do anything else with it,
			// since we've already run it through all novelty tables.
			if (!node->_processed) {
				if (nov == 2) { // i.e. the node has exactly w_{#, #r} = 2
					_stats.wgr2_node();
					process_node(node);
				} else {
					handle_unprocessed_node(node, true);
				}
			}

			return true;
		}

		///// Q_REST QUEUE /////
		// Process the rest of the nodes, i.e. those with w_{#g, #r} > 2
		// We only extract one node and process it, as this will hopefully yield nodes with low novelty
		// that will thus have more priority than the rest of nodes in this queue.
		if (!_qrest.empty()) {
			LPT_EDEBUG("multiqueue-search", "Expanding one remaining node with w_{#g, #r} > 2");
			NodePT node = _qrest.next();
			if (!node->_processed) {
				_stats.wgr_gt2_node();
				process_node(node);
			}
			return true;
		}

		return false;
	}

	inline void handle_unprocessed_node(const NodePT& node, bool is_last_queue) {
		if (is_last_queue && !_pruning) {
			_qrest.insert(node);
		}
	}


	//! When opening a node, we compute #g and evaluates whether the given node has <#g>-novelty 1 or not;
	//! if that is the case, we insert it into a special queue.
	//! Returns true iff the newly-created node is a solution
	bool create_node(const NodePT& node) {
		if (is_goal(node)) {
			LPT_INFO("search", "Goal node was found");
			_solution = node;
			return true;
		}
		node->unachieved_subgoals = _heuristic.compute_unachieved(node->state);

		if (node->unachieved_subgoals < _min_subgoals_to_reach) {
			_min_subgoals_to_reach = node->unachieved_subgoals;
			_best_found = node;
			LPT_INFO("search", "Min. # unreached subgoals: " << _min_subgoals_to_reach << "/" << _model.num_subgoals());
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

		return false;
	}

	//! Process the node. Return true iff at least one node was created during the processing.
	void process_node(const NodePT& node) {
		//assert(!node->_processed); // Don't process a node twice!
		node->_processed = true; // Mark the node as processed
		_closed.put(node);
		expand_node(node);
	}

	// Return true iff at least one node was created
	void expand_node(const NodePT& node) {
		LPT_DEBUG("cout", *node);
		_stats.expansion();
		if (node->decreases_unachieved_subgoals()) _stats.expansion_g_decrease();

		for (const auto& action:_model.applicable_actions(node->state, true)) {
			// std::cout << *(Problem::getInstance().getGroundActions()[action]) << std::endl;
			StateT s_a = _model.next(node->state, action);
			NodePT successor = std::make_shared<NodeT>(std::move(s_a), action, node, ++_generated);

            if (_closed.check(successor)) continue; // The node has already been closed
            if (is_open(successor)) continue; // The node is currently on (some) open list, so we ignore it

            // std::cout << "Generating node: " << *successor << std::endl;
            // If the node we're expanding has a monotonicity CSP, we update it
            // and "propagate" it to the children nodes
            if (_monotonicity_csp_manager) {
                assert(!node->_domains.is_null());

                successor->_domains = _monotonicity_csp_manager->generate_node(
                        node->state,
                        node->_domains,
                        successor->state,
                        _model.get_last_changeset()
                );

                if (successor->_domains.is_null()) {
                    LPT_DEBUG("cout", "\tChildren node pruned because of inconsistent monotonicity CSP: " << std::endl << "\t" << *successor);
//                    std::cout << "Releasing domains: " << get_current_memory_in_kb();
                    successor->_domains.release();
//                    _closed.put(successor);
//                    std::cout << "  vs. " << get_current_memory_in_kb() << std::endl;

                    _stats.monot_pruned();
                    continue;
                }
            }

			if (create_node(successor)) {
				break;
			}

//            std::cout << "Generated node: " << *successor << std::endl;
		}

        // Once the node is completely expanded, we can release the memory used by the domains
        node->_domains.release();
	}

	bool is_open(const NodePT& node) const {
		return _q1.contains(node) ||
		       _qwgr1.contains(node) ||
		       _qwgr2.contains(node) ||
		       _qrest.contains(node);
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
class SBFWSDriver : public drivers::Driver {
public:
	using StateT = typename StateModelT::StateT;

	//! The necessary search method
	ExitCode search(Problem& problem, const Config& config, const drivers::EngineOptions& options, float start_time) override;

protected:
	//! The stats of the search
	BFWSStats _stats;

	//! Helper methods to set up the correct template parameters
	ExitCode do_search(const StateModelT& model, const Config& config, const drivers::EngineOptions& options, float start_time);

	template <typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
	ExitCode
	do_search1(const StateModelT& model, FeatureEvaluatorT&& featureset, const Config& config, const drivers::EngineOptions& options, float start_time);
};


} } // namespaces
