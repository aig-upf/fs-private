
#pragma once

#include <stdio.h>
#include <unordered_set>

#include <lapkt/tools/resources_control.hxx>
#include <lapkt/tools/logging.hxx>

#include <problem.hxx>
#include "base.hxx"
#include "stats.hxx"
#include <search/drivers/sbfws/relevant_atomset.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/actions.hxx>
#include <lapkt/search/components/open_lists.hxx>

#include <algorithm>

namespace fs0 { namespace bfws {
	
template <typename StateT, typename ActionType>
class IDWRunNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IDWRunNode<StateT, ActionT>>;

	//! The state in this node
	StateT state;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;
	
	//! Accummulated cost
	unsigned g;
	
	
	//! The novelty  of the state
	unsigned _w;
	
	//! The generation order, uniquely identifies the node
	//! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
	uint32_t _gen_order;


	IDWRunNode() = default;
	~IDWRunNode() = default;
	IDWRunNode(const IDWRunNode&) = default;
	IDWRunNode(IDWRunNode&&) = delete;
	IDWRunNode& operator=(const IDWRunNode&) = delete;
	IDWRunNode& operator=(IDWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	IDWRunNode(const StateT& s, unsigned long gen_order) : IDWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	IDWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, uint32_t gen_order) :
		state(std::move(_state)),
		action(_action),
		parent(_parent),
		g(parent ? parent->g+1 : 0),
		_w(std::numeric_limits<unsigned>::max()),
		_gen_order(gen_order)
	{
		assert(_gen_order > 0); // Very silly way to detect overflow, in case we ever generate > 4 billion nodes :-)
	}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IDWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
// 		const Problem& problem = Problem::getInstance();
		os << "{@ = " << this;
		os << ", #=" << _gen_order ;
		os << ", s = " << state ;
		os << ", g=" << g ;
		//os << ", w=" << (_evaluated ? (_w == std::numeric_limits<unsigned>::max() ? "INF" : std::to_string(_w)) : "?") ;
		os << ", w=" << (_w == std::numeric_limits<unsigned>::max() ? "INF" : std::to_string(_w));
		
		os << ", act=" << action ;
		
		os << ", parent = " << (parent ? "#" + std::to_string(parent->_gen_order) : "None");
		return os;
	}

	bool operator==( const IDWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


template <typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class IDWSimulationEvaluator {
protected:
	//! The set of features used to compute the novelty
	const FeatureSetT& _features;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	std::unique_ptr<NoveltyEvaluatorT> _evaluator;

public:
	IDWSimulationEvaluator(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
		_features(features),
		_evaluator(evaluator)
	{}

	~IDWSimulationEvaluator() = default;
	
	//! Returns false iff we want to prune this node during the search
	/*
	unsigned evaluate(NodeT& node) {
		if (node.parent) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			node._w = _evaluator->evaluate(_features.evaluate(node.state), _features.evaluate(node.parent->state));
		} else {
			node._w = _evaluator->evaluate(_features.evaluate(node.state));
		}
		
		if (node._w == 2) {
			_evaluator->mark_nov2atoms_from_last_state(node._nov2_pairs);
		}
		
		return node._w;
	}
	*/
	
	unsigned char evaluate(const State& state) {
		return _evaluator->evaluate(_features.evaluate(state));
	}
	
	unsigned char evaluate(const State& state, const State& parent) {
		return _evaluator->evaluate(_features.evaluate(state), _features.evaluate(parent));
	}		
	
	
	std::vector<bool> reached_atoms() const {
		std::vector<bool> atoms;
		_evaluator->mark_atoms_in_novelty1_table(atoms);
		return atoms;
	}
	
	void reset() {
		_evaluator->reset();
	}			
	
};


//! A single Iterative-Deepening Width-Pruned run (with parametrized max. width) that runs until (independent)
template <typename NodeT,
          typename StateModel,
          typename NoveltyEvaluatorT,
		  typename FeatureSetT
>
class IDWRun
{
public:
	using ActionT = typename StateModel::ActionType;
	using StateT = typename StateModel::StateT;
	
	using ActionIdT = typename StateModel::ActionType::IdType;
	using NodePT = std::shared_ptr<NodeT>;
	
	using SimEvaluatorT = IDWSimulationEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
	
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;
	
	using OpenListT = lapkt::SimpleQueue<NodeT>;
	
	struct Config {
		//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
		bool _complete;
		
		//! Whether to take into account negative propositions or not
		bool _mark_negative;
		
		//! The maximum levels of novelty to be considered
		unsigned _max_width;
		
		//! Whether to extract goal-informed relevant sets R
		bool _goal_directed;
		
		//!
		bool _force_hybrid_run;
		
		Config(bool complete, bool mark_negative, unsigned max_width, bool goal_directed_info, bool force_hybrid_run) :
			_complete(complete), _mark_negative(mark_negative), _max_width(max_width), _goal_directed(goal_directed_info), _force_hybrid_run(force_hybrid_run) {}
	};
	
protected:
	//! The search model
	const StateModel& _model;
	
	//! The open list
	OpenListT _open;
	
	
	//! The simulation configuration
	Config _config;

	// TODO REMOVE ?
 	std::vector<NodePT> _optimal_paths;
	
	std::vector<bool> _reached_subgoals;
	std::vector<bool> _reached_subgoals_across_different_runs;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	// TODO REMOVE ?
	std::unordered_set<unsigned> _unreached;
	
	//! Contains the indexes of all those goal atoms that were already reached in the seed state
	std::vector<bool> _in_seed;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	SimEvaluatorT _evaluator;
	
	//! Some node counts
	uint32_t _generated;
	uint32_t _w1_nodes_expanded;
	uint32_t _w2_nodes_expanded;
	uint32_t _w1_nodes_generated;
	uint32_t _w2_nodes_generated;
	uint32_t _w_gt2_nodes_generated;
	
	BFWSStats& _stats;
	
	//! Whether to print some useful extra information or not
	bool _verbose;
	
	unsigned _width_bound;
	unsigned _depth_bound;
	
	
	unsigned long _pruned_by_width;
	unsigned long _pruned_by_depth;	
	
	bool _depth_bound_reached;
	
	std::vector<StateT> _state_stack;	

public:

	//! Constructor
	IDWRun(const StateModel& model, const FeatureSetT& featureset, NoveltyEvaluatorT* evaluator, const IDWRun::Config& config, BFWSStats& stats, bool verbose) :
// 		Base(model, OpenListT(), ClosedListT()),
		_model(model),
		_open(),
		_config(config),
		_optimal_paths(model.num_subgoals()),
		_reached_subgoals(model.num_subgoals()),
		_reached_subgoals_across_different_runs(model.num_subgoals()),
		_unreached(),
		_in_seed(model.num_subgoals(), false),
		_evaluator(featureset, evaluator),
		_generated(1),
		_w1_nodes_expanded(0),
		_w2_nodes_expanded(0),
		_w1_nodes_generated(0),
		_w2_nodes_generated(0),		
		_w_gt2_nodes_generated(0),
		_stats(stats),
		_verbose(verbose),
		_pruned_by_width(0),
		_pruned_by_depth(0)
	{
	}
	
	void reset() {
		std::vector<NodePT> _(_optimal_paths.size(), nullptr);
		_optimal_paths.swap(_);
		_generated = 1;
// 		_w1_nodes_expanded = 0;
// 		_w2_nodes_expanded = 0;
// 		_w1_nodes_generated = 0;
// 		_w2_nodes_generated = 0;		
// 		_w_gt2_nodes_generated = 0;
		_evaluator.reset();
	}	

	~IDWRun() = default;

	// Disallow copy, but allow move
	IDWRun(const IDWRun&) = delete;
	IDWRun(IDWRun&&) = default;
	IDWRun& operator=(const IDWRun&) = delete;
	IDWRun& operator=(IDWRun&&) = default;

	/*
	//! Mark all atoms in the path to some goal. 'seed_nodes' contains all nodes satisfying some subgoal.
	void mark_atoms_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes, std::vector<bool>& atoms) const {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::unordered_set<NodePT> all_visited;
		assert(atoms.size() == index.size());
		
		for (NodePT node:seed_nodes) {
			
			NodePT root = node;
			// We ignore s0
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				auto res = all_visited.insert(node);
				if (!res.second) break;
				
				const StateT& state = node->state;
				const StateT& parent_state = node->parent->state;
				_unused(state);
				
				for (const auto& p_q:node->_nov2_pairs) {
					Atom p = index.to_atom(p_q.first);
					Atom q = index.to_atom(p_q.second);
					
					assert(state.contains(p));
					assert(state.contains(q));
					assert(!(parent_state.contains(p) && parent_state.contains(q))); // Otherwise the tuple couldn't be new
					
					
					std::vector<AtomIdx> to_mark;
					if (parent_state.contains(p)) {
						if (p.getValue() != 0) atoms[p_q.first] = true; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
					}
					else if (parent_state.contains(q)) {
						if (q.getValue() != 0) atoms[p_q.second] = true;
					}
					else { // The parent state contains none
						if (p.getValue() != 0) atoms[p_q.first] = true;
						if (q.getValue() != 0) atoms[p_q.second] = true;
					}
					
				}
				
				node = node->parent;
			}			
		}
	}
	
	
	std::vector<bool> compute_R(const StateT& seed) {

		_config._complete = false;
		
		float simt0 = aptk::time_used();
 		run(seed);
		_stats.simulation();
		_stats.sim_add_time(aptk::time_used() - simt0);
		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
		_stats.reachable_subgoals(_model.num_subgoals() - _unreached.size());
		
		if (_config._goal_directed) {
			// If we want the goal-aware R_G, we compute it, and use it _unless_ it is too small, in which case we fall back to the goal-unaware R.
			unsigned R_G_size;
			std::vector<bool> R_G = extract_R_G(R_G_size);
// 			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " seed nodes)");
			
			unsigned subgoals = _model.num_subgoals();
			if (R_G_size > 0 && R_G_size > std::min(int(subgoals*0.5), 4)) {
				return R_G;
			} else {
				LPT_INFO("cout", "R_G too small, falling back to goal-unaware R");
			}
		}
		
		
		// Compute the goal-unaware version of R containing all atoms seen during the IW run
		std::vector<bool> R = _evaluator.reached_atoms();
		if (_verbose) {
			LPT_INFO("cout", "Simulation - |R[" << _config._max_width << "]| = " << std::count(R.begin(), R.end(), true));
		}
		return R;
	}
	
	//! Extractes the goal-oriented set of relevant atoms after a simulation run
	std::vector<bool> extract_R_G(unsigned& R_G_size) {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();		

		std::vector<NodePT> seed_nodes;
		for (unsigned subgoal_idx = 0; subgoal_idx < _optimal_paths.size(); ++subgoal_idx) {
			if (!_in_seed[subgoal_idx] && _optimal_paths[subgoal_idx] != nullptr) {
				seed_nodes.push_back(_optimal_paths[subgoal_idx]);
			}
		}		

		if (_verbose && !_unreached.empty()) {
			LPT_INFO("cout", "WARNING: Some subgoals not reached during the simulation.");
// 			for (unsigned x:_unreached) LPT_INFO("cout", "\t Unreached subgoal idx: " << x);
		}
		
		std::vector<bool> R_G(index.size(), false);
		mark_atoms_in_path_to_subgoal(seed_nodes, R_G);
		
		R_G_size = std::count(R_G.begin(), R_G.end(), true);
		if (_verbose) {
			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " seed nodes)");
		}
		
		return R_G;		
	}	
	*/
	
	std::vector<bool> compute_R(const StateT& seed) {
		
		float simt0 = aptk::time_used();
		run(seed);
		report_simulation_stats(simt0);
		return {};
	}
	
	
	void report_simulation_stats(float simt0) {
		unsigned globally_reached = std::count(_reached_subgoals_across_different_runs.cbegin(),_reached_subgoals_across_different_runs.cend(), true);
		_stats.simulation();
		_stats.sim_add_time(aptk::time_used() - simt0);
		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
		_stats.global_reachable_subgoals(globally_reached);
	}
	
	void run(const StateT& state) {
	
		_state_stack.reserve(1000);
		mark_seed_subgoals(state);
		
	
		bool all_subgoals_found_in_one_run = false;
		
		for (unsigned w = 1; w <= 2 && !all_subgoals_found_in_one_run; ++w) {
			
			_width_bound = w;
			LPT_INFO("cout", "IDWDFS Simulation - Width bound set to " << _width_bound);

			unsigned num_times_depth_bound_unreached = 0;
			
			for (unsigned d = 4; d <= 1000 && !all_subgoals_found_in_one_run; ++d) {
				
				_depth_bound = d;
				_depth_bound_reached = false;
				
				if (d % 10 == 0)
					LPT_INFO("cout", "IDWDFS Simulation - Depth bound set to " << _depth_bound);
				
				// Reset the data structures
				reset();
				
				_state_stack.push_back(state);
				all_subgoals_found_in_one_run = idwdfs(1);
				_state_stack.pop_back();
				assert(_state_stack.empty()); // Make sure the stack was properly cleared and only the seed remains there
				
				
				// If the depth bound wasn't reached with the current width bound, we'll increase it;
				// otherwise we'll increase the depth bound
				if (!_depth_bound_reached) {
					if (++num_times_depth_bound_unreached >= 4) break;
				}
			}
			
		}
		
		if (all_subgoals_found_in_one_run) {
			report("All subgoals found!");
		} else {
			report("State space exhausted");
		}
		
	}
	
	
	//! Evaluates the novelty on the node at the top of the current stack
	unsigned char evaluate_novelty() {
		unsigned depth = _state_stack.size();
		assert(depth >= 1);
		StateT& current = _state_stack.back();
		
		if (depth > 1) {
			return _evaluator.evaluate(current, _state_stack[depth-2]);
		} else {
			return _evaluator.evaluate(current);
		}
	}
	
	bool idwdfs(unsigned depth) {
		StateT& current = _state_stack.back();
		
		// Base case: depth bound has been reached
		if (depth > _depth_bound) {
			_depth_bound_reached = true;
			_pruned_by_depth++;
// 			LPT_DEBUG("cout", "IDWDFS Simulation - Depth bound reached");
			return false;
		}
		
		// Check whether the node satisfies some subgoal
		bool one_subgoal_reached, all_subgoals_reached;
		process_node(current, one_subgoal_reached, all_subgoals_reached);
		if (all_subgoals_reached) {
			return true;
		}
		
		// Check whether we need to prune the node
		unsigned char novelty = evaluate_novelty();
		update_novelty_counters_on_generation(novelty);
		if (novelty > _width_bound) {
			_pruned_by_width++;
// 			LPT_DEBUG("cout", "IDWDFS Simulation - Width bound reached");
			return false;
		}
		
			
		// Else, expand the node recursively
		update_novelty_counters_on_expansion(novelty);
		
		std::vector<typename StateModel::ActionType::IdType> applicable;
		for (const auto& a : _model.applicable_actions(current)) applicable.push_back(a);
		std::random_shuffle( applicable.begin(), applicable.end() );
		
		for (const auto& a : applicable) {
		//for (const auto& a : _model.applicable_actions(current)) {
			StateT successor = _model.next(current, a);
			_generated++;
			// LPT_INFO("cout", "Simulation - Node generated: " << *successor);
			
			_state_stack.push_back(successor);
			bool res = idwdfs(depth + 1);
			_state_stack.pop_back();
			if (res) {
// 				m_plan.push_back(a);
				return true;
			}
		}
		
		return false;
	}
	
	
	void report(const std::string& result) const {
		if (!_verbose) return;
		LPT_INFO("cout", "Simulation - Result: " << result);
		LPT_INFO("cout", "Simulation - Num reached subgoals: " << (_model.num_subgoals() - _unreached.size()) << " / " << _model.num_subgoals());
		LPT_INFO("cout", "Simulation - Nodes depth-pruned: " << _pruned_by_depth);
		LPT_INFO("cout", "Simulation - Nodes width-pruned: " << _pruned_by_width);
		
		
		LPT_INFO("cout", "Simulation - Num subgoals reached across different runs: " <<
					std::count(_reached_subgoals_across_different_runs.cbegin(),_reached_subgoals_across_different_runs.cend(), true));
		
		LPT_INFO("cout", "Simulation - Expanded nodes with w=1 " << _w1_nodes_expanded);
		LPT_INFO("cout", "Simulation - Expanded nodes with w=2 " << _w2_nodes_expanded);		
		LPT_INFO("cout", "Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
		LPT_INFO("cout", "Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
		LPT_INFO("cout", "Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);
	}

	void update_novelty_counters_on_expansion(unsigned char novelty) {
		if (novelty == 1) ++_w1_nodes_expanded;
		else if (novelty== 2) ++_w2_nodes_expanded;
	}
	
	void update_novelty_counters_on_generation(unsigned char novelty) {
		if (novelty==1) ++_w1_nodes_generated;
		else if (novelty==2)  ++_w2_nodes_generated;
		else ++_w_gt2_nodes_generated;
	}
	
protected:

	//! Returns true iff all goal atoms have been reached in the IW search
	void process_node(const StateT& state, bool& one_subgoal_reached, bool& all_subgoals_reached) {
		
		// We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
		// to check if the current node satisfies any of them - and if it does, we mark it appropriately.
		for (auto it = _unreached.begin(); it != _unreached.end(); ) {
			unsigned subgoal_idx = *it;

			if (_model.goal(state, subgoal_idx)) {
// 				if (!_optimal_paths[subgoal_idx]) _optimal_paths[subgoal_idx] = node;
				if (!_reached_subgoals[subgoal_idx]) {
					_reached_subgoals[subgoal_idx] = true;
					one_subgoal_reached = true;
				}
				_reached_subgoals_across_different_runs[subgoal_idx] = true;
				it = _unreached.erase(it);
			} else {
				++it;
			}
		}
		
		/*
		// Check whether the node is actually a goal state...!
		bool any_unreached = false;
		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (!_model.goal(state, i)) {
				any_unreached = true;
				break;
			}
		}
		if (!any_unreached) throw std::runtime_error("Goal State Found!");
		*/
		
		all_subgoals_reached = _unreached.empty();
	}
	
	
	void mark_seed_subgoals(const StateT& state) {
		_reached_subgoals = std::vector<bool>(_model.num_subgoals(), false);
		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (_model.goal(state, i)) {
				_in_seed[i] = true;
				_reached_subgoals[i] = true;
				_reached_subgoals_across_different_runs[i] = true;
			} else {
				_unreached.insert(i);
			}
		}
	}	

// public:
// 	const std::unordered_set<NodePT>& get_relevant_nodes() const { return _visited; }
};

} } // namespaces
