
#pragma once

#include <fs/core/search/drivers/sbfws/iw_run.hxx>
#include <fs/core/search/drivers/registry.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/heuristics/unsat_goal_atoms.hxx>
#include <fs/core/heuristics/l0.hxx>
#include <lapkt/tools/resources_control.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>

#include <fs/core/search/drivers/sbfws/stats.hxx>
#include <fs/core/search/drivers/sbfws/relevant_atoms.hxx>
#include <fs/core/constraints/gecode/handlers/monotonicity_csp.hxx>


namespace fs0::bfws {

using Novelty = lapkt::novelty::Novelty;

//! Prioritize nodes with lower w_{#g, r}, then lower number of unachieved goals #g; break ties
//! with cost of path so far (g) and, eventually, with generation order.
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

	//! The generation order, uniquely identifies the node
	//! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
	uint32_t _gen_order;

	//! The numeric value of the novelty w_{#g,#r}
	unsigned short w_g_r;

	//! A reference atomset helper wrt which the sets R of descendent nodes with same #g are computed
	//! Use a raw pointer to optimize performance, as the number of generated nodes will typically be huge
	AtomsetHelper* _helper;

	//! The number of atoms in the last relaxed plan computed in the way to the current state that have been
	//! made true along the path (#r)
	//! Use a raw pointer to optimize performance, as the number of generated nodes will typically be huge
	RelevantAtomSet* _relevant_atoms;

    //! The sets D^G_X of goal-reachable domains for every state variable X
    DomainTracker _domains;

	//! Constructor with full copying of the state (expensive)
	SBFWSNode(const StateT& s, unsigned long gen_order) : SBFWSNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	SBFWSNode(StateT&& _state, action_t action_, ptr_t parent_, uint32_t gen_order) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0),
		unachieved_subgoals(std::numeric_limits<unsigned>::max()),
		_gen_order(gen_order),
		_helper(nullptr),
		_relevant_atoms(nullptr)
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
			reached = std::to_string(_relevant_atoms->num_reached()) + "/" + std::to_string(_relevant_atoms->getHelper()._num_relevant);
		} else {
			reached = std::to_string(0);
		}
		os << "#" << _gen_order << " (" << this << "), " << state;
		os << ", g = " << g <<  ", w_gr=" << w_g_r << ", #g=" << unachieved_subgoals << ", #r=" << reached;
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



template <typename StateModelT, typename NoveltyIndexerT, typename FeatureSetT, typename NoveltyEvaluatorT, typename NodeT>
class SBFWSHeuristic {
public:
	using NoveltyEvaluatorMapT = std::unordered_map<long, NoveltyEvaluatorT*>;
	using ActionT = typename StateModelT::ActionType;
    using IWNodeT = IWRunNode<State, ActionT>;
    using SimulationT = IWRun<IWNodeT, StateModelT, NoveltyEvaluatorT, FeatureSetT>;
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;


protected:
	const StateModelT& _model;

	const Problem& _problem;

	const FeatureSetT& _featureset;

	const NoveltyFactory<FeatureValueT> _search_novelty_factory;

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
		_wgr_novelty_evaluators(3), // We'll only care about novelties 1 and, at most, 2.
		_unsat_goal_atoms_heuristic(_problem),
		_mark_negative_propositions(config.mark_negative_propositions),
        _stats(stats),
        _sbfwsconfig(config),
        _r_counter(RelevantAtomsCounterFactory::build<StateModelT, NodeT, SimulationT, NoveltyEvaluatorT, FeatureSetT>(model, config, features))
    {
    }

	~SBFWSHeuristic() {
		for (auto& elem:_wgr_novelty_evaluators) for (auto& p:elem) delete p.second;
	};


	unsigned get_hash_r(NodeT& node) {
		return _r_counter->count(node, _stats);
	}

	unsigned compute_node_complex_type(NodeT& node) {
// 		LPT_INFO("types", "Type=" << compute_node_complex_type(node.unachieved_subgoals, get_hash_r(node)) << " for node: " << std::endl << node)
// 		LPT_INFO("hash_r", "#r=" << get_hash_r(node) << " for node: " << std::endl << node)
		return compute_node_complex_type(node.unachieved_subgoals, get_hash_r(node));
	}

	unsigned evaluate_wgr1(NodeT& node) {
		unsigned type = compute_node_complex_type(node);
		unsigned ptype = node.has_parent() ? compute_node_complex_type(*(node.parent)) : 0;
		return evaluate_novelty(node, _wgr_novelty_evaluators, 1, type, ptype);
	}

	unsigned evaluate_wgr2(NodeT& node) {
		unsigned type = compute_node_complex_type(node);
		unsigned ptype = node.has_parent() ? compute_node_complex_type(*(node.parent)) : 0;
		return evaluate_novelty(node, _wgr_novelty_evaluators, 2, type, ptype);
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

	unsigned evaluate_novelty(const NodeT& node, std::vector<NoveltyEvaluatorMapT>& evaluator_map, unsigned k, unsigned type, unsigned parent_type) {
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
template <typename StateModelT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SBFWS {
public:
	using StateT = typename StateModelT::StateT;
	using ActionT = typename StateModelT::ActionType;
	using ActionIdT = typename ActionT::IdType;
	using NodeT = SBFWSNode<fs0::State, ActionT>;
	using PlanT =  std::vector<ActionIdT>;
	using NodePT = std::shared_ptr<NodeT>;
	using ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>;
    using HeuristicT = SBFWSHeuristic<StateModelT, SBFWSNoveltyIndexer, FeatureSetT, NoveltyEvaluatorT, NodeT>;
	using SimulationNodeT = typename HeuristicT::IWNodeT;


protected:

	//! An open list sorted by the numerical value of width, then #g
	using NoveltyComparerT = novelty_comparer<NodePT>;
	using StandardOpenList = lapkt::UpdatableOpenList<NodeT, NodePT, NoveltyComparerT>;

	using SearchableQueue = lapkt::SearchableQueue<NodeT>;


	//! The search model
	const StateModelT& _model;

	//! The solution node, if any. This will be set during the search process
	NodePT _solution;

	StandardOpenList _open;


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

        LPT_INFO("cout", "Mem. usage on start of SBFWS search: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");

        NodePT root = std::make_shared<NodeT>(s, ++_generated);

        if (_monotonicity_csp_manager) {
            root->_domains = _monotonicity_csp_manager->create_root(s);
            if (root->_domains.is_null()) {
                LPT_INFO("cout", "Root node detected as inconsistent for monotonicity reasons");
                _stats.monot_pruned();
                return false;
            } else {
                LPT_INFO("cout", "Root node is monotonicity-consistent!");
            }
        }

        // Note that in general, the root node will have novelty 1, unless we are ignoring negative literals and
        // the initial state happens to be the empty set, i.e. the state where no atom holds.
        create_node(root);

		// Force one simulation from the root node and abort the search
//        _heuristic.get_hash_r(*root);
//  		_heuristic.compute_R(*root);
// 		return false;


		// The main search loop
		_solution = nullptr; // Make sure we start assuming no solution found

//		for (bool remaining_nodes = true; !_solution && remaining_nodes;) {
//			remaining_nodes = process_one_node();
//		}

		while (!_open.empty() && !_solution) {
			auto node = _open.next();
			process_node(node);
		}

		return extract_plan(_solution, plan);
	}

protected:


	//! When opening a node, we compute #g and evaluate whether the given node has <#g>-novelty 1 or not;
	//! if that is the case, we insert it into a special queue.
	//! Returns true iff the newly-created node is a solution
	bool create_node(const NodePT& node) {
		if (is_goal(node)) {
			LPT_INFO("search", "Goal node was found");
			_solution = node;
			return true;
		}

		// Compute #g upfront
		node->unachieved_subgoals = _heuristic.compute_unachieved(node->state);

		// Print some stats if a new low in number of unreached subgoals has been reached
		if (node->unachieved_subgoals < _min_subgoals_to_reach) {
			_min_subgoals_to_reach = node->unachieved_subgoals;
			LPT_INFO("search", "Min. # unreached subgoals: " << _min_subgoals_to_reach << "/" << _model.num_subgoals());
		}

		node->w_g_r = 999;
		unsigned nov = _heuristic.evaluate_wgr1(*node);
		if (nov == 1) {
			node->w_g_r = 1;

		} else if (_novelty_levels == 3) {
			if (_heuristic.evaluate_wgr2(*node) == 2) {
				node->w_g_r = 2;
			}
		}

        if (node->w_g_r == 1) _stats.wgr1_node();
        else if (node->w_g_r == 2) _stats.wgr2_node();
        else _stats.wgr_gt2_node();


        _open.insert(node);


		if (node->decreases_unachieved_subgoals()) _stats.generation_g_decrease();

		return false;
	}

	//! Process the node.
	void process_node(const NodePT& node) {
		_closed.put(node);
		expand_node(node);
	}

    float node_generation_rate() {
        return _stats.generated() * 1.0 / (aptk::time_used() - _stats.initial_search_time());
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

			_stats.generation();
            auto generated = _stats.generated();
            if (generated % 50000 == 0) {

                LPT_INFO("cout", "Node generation rate after " << generated / 1000 << "K generations (nodes/sec.): " << node_generation_rate()
                        << ". Memory consumption: "<< get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
            }

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
                    successor->_domains.release();
                    _stats.monot_pruned();
//                    _closed.put(successor);
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
		return _open.contains(node);
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


} // namespaces
