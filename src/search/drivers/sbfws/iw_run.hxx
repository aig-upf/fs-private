
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



namespace fs0 { namespace bfws {
	
template <typename StateT, typename ActionType>
class IWRunNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IWRunNode<StateT, ActionT>>;

	//! The state in this node
	StateT state;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;
	
	//! Accummulated cost
	unsigned g;
	
// 	bool satisfies_subgoal; // Whether the node satisfies some subgoal
	
	//! The novelty  of the state
	unsigned char _w;
	
	//! The indexes of the variables whose atoms form the set 1(s), which contains all atoms in 1(parent(s)) not deleted by the action that led to s, plus those 
	//! atoms in s with novelty 1.
// 	std::vector<unsigned> _nov1atom_idxs;
	
	//! Implicit encoding of the atoms that contribute novelty 1 to the state
// 	boost::dynamic_bitset<> _B_of_s;
	
	//! Whether the path-novely of the node is one
// 	bool _path_novelty_is_1;
	
	std::vector<std::pair<AtomIdx, AtomIdx>> _nov2_pairs;
	
	//! The generation order, uniquely identifies the node
	//! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
	uint32_t _gen_order;


	IWRunNode() = default;
	~IWRunNode() = default;
	IWRunNode(const IWRunNode&) = default;
	IWRunNode(IWRunNode&&) = delete;
	IWRunNode& operator=(const IWRunNode&) = delete;
	IWRunNode& operator=(IWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	IWRunNode(const StateT& s, unsigned long gen_order) : IWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	IWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, uint32_t gen_order) :
		state(std::move(_state)),
//		feature_valuation(0),
		action(_action),
		parent(_parent),
		g(parent ? parent->g+1 : 0),
		_w(std::numeric_limits<unsigned char>::max()),
// 		_path_novelty_is_1(false),
// 		_B_of_s(state.numAtoms()),
		_gen_order(gen_order)
	{
		assert(_gen_order > 0); // Very silly way to detect overflow, in case we ever generate > 4 billion nodes :-)
	}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
// 		const Problem& problem = Problem::getInstance();
		os << "{@ = " << this;
		os << ", #=" << _gen_order ;
		os << ", s = " << state ;
		os << ", g=" << g ;
		//os << ", w=" << (_evaluated ? (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w)) : "?") ;
		os << ", w=" << (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w));
		
		os << ", act=" << action ;
// 		if (action < std::numeric_limits<unsigned>::max()) {
// 			os << ", act=" << *(problem.getGroundActions()[action]) ;
// 			os << fs0::print::full_action(*(problem.getGroundActions()[action]));
// 		} else {
// 			os << ", act=" << "NONE" ;
// 		}
		
		os << ", parent = " << (parent ? "#" + std::to_string(parent->_gen_order) : "None");
		return os;
	}

	bool operator==( const IWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


template <typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class SimulationEvaluator {
protected:
	//! The set of features used to compute the novelty
	const FeatureSetT& _features;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	std::unique_ptr<NoveltyEvaluatorT> _evaluator;

public:
	SimulationEvaluator(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
		_features(features),
		_evaluator(evaluator)
	{}

	~SimulationEvaluator() = default;
	
	//! Returns false iff we want to prune this node during the search
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
	
	std::vector<bool> reached_atoms() const {
		std::vector<bool> atoms;
		_evaluator->mark_atoms_in_novelty1_table(atoms);
		return atoms;
	}
	
	void reset() {
		_evaluator->reset();
	}		
	
};


//! A single IW run (with parametrized max. width) that runs until (independent)
//! satisfaction of each of the provided goal atoms, and computes the set
//! of atoms R that is relevant for the achievement of at least one atom.
//! R is computed treating the actions as a black-box. For this, an atom is considered
//! relevant for a certain goal atom if that atom is true in at least one of the states
//! that lies on the path between the seed node and the first node where that goal atom is
//! satisfied.
template <typename NodeT,
          typename StateModel,
          typename NoveltyEvaluatorT,
		  typename FeatureSetT
>
class IWRun
{
public:
	using ActionT = typename StateModel::ActionType;
	using StateT = typename StateModel::StateT;
	
	using ActionIdT = typename StateModel::ActionType::IdType;
	using NodePT = std::shared_ptr<NodeT>;
	
	using SimEvaluatorT = SimulationEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
	
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
		bool _force_adaptive_run;
		
		Config(bool complete, bool mark_negative, unsigned max_width, bool goal_directed_info, bool force_adaptive_run) :
			_complete(complete), _mark_negative(mark_negative), _max_width(max_width), _goal_directed(goal_directed_info), _force_adaptive_run(force_adaptive_run) {}
	};
	
protected:
	//! The search model
	const StateModel& _model;
	
	//! The simulation configuration
	Config _config;

	//!
	std::vector<NodePT> _optimal_paths;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
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
	
	//! The general statistics of the search
	BFWSStats& _stats;
	
	//! Whether to print some useful extra information or not
	bool _verbose;

public:

	//! Constructor
	IWRun(const StateModel& model, const FeatureSetT& featureset, NoveltyEvaluatorT* evaluator, const IWRun::Config& config, BFWSStats& stats, bool verbose) :
		_model(model),
		_config(config),
		_optimal_paths(model.num_subgoals()),
		_unreached(),
		_in_seed(),
		_evaluator(featureset, evaluator),
		_generated(1),
		_w1_nodes_expanded(0),
		_w2_nodes_expanded(0),
		_w1_nodes_generated(0),
		_w2_nodes_generated(0),		
		_w_gt2_nodes_generated(0),
		_stats(stats),
		_verbose(verbose)		
	{
	}
	
	void reset() {
		std::vector<NodePT> _(_optimal_paths.size(), nullptr);
		_optimal_paths.swap(_);
		_generated = 1;
		_w1_nodes_expanded = 0;
		_w2_nodes_expanded = 0;
		_w1_nodes_generated = 0;
		_w2_nodes_generated = 0;		
		_w_gt2_nodes_generated = 0;
		_evaluator.reset();
	}

	~IWRun() = default;

	// Disallow copy, but allow move
	IWRun(const IWRun&) = delete;
	IWRun(IWRun&&) = default;
	IWRun& operator=(const IWRun&) = delete;
	IWRun& operator=(IWRun&&) = default;

	
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
						if (p.getValue() != 0) {
							atoms[p_q.first] = true; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
// 							std::cout << "ATTENTION : Marking atom " << p << std::endl;
						}
					}
					else if (parent_state.contains(q)) {
						if (q.getValue() != 0) {
							atoms[p_q.second] = true;
// 							std::cout << "ATTENTION : Marking atom " << q << std::endl;
						}
					}
					else { // The parent state contains none
// 						std::cout << "ATTENTION : Would Mark pair " << p << ", " << q << std::endl;
						if (p.getValue() != 0) atoms[p_q.first] = true;
						if (q.getValue() != 0) atoms[p_q.second] = true;
					}
					
				}
				
				/*
				const StateT& state = node->state;
				for (unsigned var = 0; var < state.numAtoms(); ++var) {
					if (state.getValue(var) == 0) continue; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
					AtomIdx atom = index.to_index(var, state.getValue(var));
					atoms[atom] = true;
				}
				*/
				
				node = node->parent;
			}			
		}
	}
	
	void mark_w1_atoms_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes, std::vector<bool>& atoms) const {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::unordered_set<NodePT> all_visited;
		assert(atoms.size() == index.size());
		
		for (NodePT node:seed_nodes) {
			// We ignore s0
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				auto res = all_visited.insert(node);
				if (!res.second) break;
				
				const StateT& state = node->state;
				for (unsigned var = 0; var < state.numAtoms(); ++var) {
					ObjectIdx val = state.getValue(var);
					if (val != 0) {
						atoms[index.to_index(var, val)] = true;
					}
				}
				
				node = node->parent;
			}			
		}
	}	
	
	void report_simulation_stats(float simt0) {
		_stats.simulation();
		_stats.sim_add_time(aptk::time_used() - simt0);
		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
	}
	
	std::vector<bool> compute_R(const StateT& seed) {
		if (_config._force_adaptive_run) {
			return compute_adaptive_R(seed);
		} else if (_config._max_width == 1){
			return compute_plain_R1(seed);
		} else if (_config._max_width == 2){
			return compute_plain_RG2(seed);
		} else {
			throw std::runtime_error("Simulation max_width too high");
		}
	}
	
	std::vector<bool> compute_plain_RG2(const StateT& seed) {
		assert(_config._max_width == 2);
		
		_config._complete = false;
		float simt0 = aptk::time_used();
  		run(seed, _config._max_width);
		report_simulation_stats(simt0);
		
		LPT_INFO("cout", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
		return extract_R_G(true);
	}
	
	
	std::vector<bool> compute_plain_R1(const StateT& seed) {
		assert(_config._max_width == 1);
		_config._complete = false;
		
		float simt0 = aptk::time_used();
  		run(seed, _config._max_width);
		report_simulation_stats(simt0);
		
		if (_config._goal_directed && _unreached.size() == 0) {
			LPT_INFO("cout", "Simulation - IW(" << _config._max_width << ") reached all subgoals, computing R_G[" << _config._max_width << "]");
			return extract_R_G_1();
		}
		
		// Else, compute the goal-unaware version of R containing all atoms seen during the IW run
		std::vector<bool> R = _evaluator.reached_atoms();
		LPT_INFO("cout", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
		if (_verbose) {
			unsigned c = std::count(R.begin(), R.end(), true);
			LPT_INFO("cout", "Simulation - |R[" << _config._max_width << "]| = " << c);
			_stats.relevant_atoms(c);
		}
		return R;
	}	
	
	std::vector<bool> compute_adaptive_R(const StateT& seed) {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		_config._complete = false;
		
		float simt0 = aptk::time_used();
  		run(seed, 1);
		report_simulation_stats(simt0);
		
		if (_unreached.size() == 0) {
			// If a single IW[1] run reaches all subgoals, we return R=emptyset
			LPT_INFO("cout", "Simulation - IW(1) run reached all goals, thus R={}");
			_stats.r_type(0);
			return std::vector<bool>(index.size(), false);
		} else {
			LPT_INFO("cout", "Simulation - IW(1) run did not reach all goals, throwing IW(2) simulation");
		}
		
		// Otherwise, run IW(2)
		reset();
		run(seed, 2);
		report_simulation_stats(simt0);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
		
		return extract_R_G(true);
	}	
	
	//! Extracts the goal-oriented set of relevant atoms after a simulation run
	std::vector<bool> extract_R_G(bool r_all_fallback) {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		/*
		for (unsigned subgoal_idx = 0; subgoal_idx < _all_paths.size(); ++subgoal_idx) {
			const std::vector<NodePT>& paths = _all_paths[subgoal_idx];
			assert(_in_seed[subgoal_idx] || !paths.empty());
			seed_nodes.insert(seed_nodes.end(), paths.begin(), paths.end());
		}
		*/

		
		if (r_all_fallback) {
			unsigned num_subgoals = _model.num_subgoals();
			unsigned initially_reached = std::count(_in_seed.begin(), _in_seed.end(), true);
			unsigned reached_by_simulation = num_subgoals - _unreached.size() - initially_reached;
			if (_verbose) LPT_INFO("cout", "Simulation - " << reached_by_simulation << " subgoals were newly reached by the simulation.");
			bool decide_r_all = (reached_by_simulation < (0.5*num_subgoals));
			if (decide_r_all) {
				if (_verbose) LPT_INFO("cout", "Simulation - Falling back to R=R[All]");	
				_stats.r_type(1);
				return std::vector<bool>(index.size(), true);
			} else {
				if (_verbose) LPT_INFO("cout", "Simulation - Computing R_G");
			}
		}
		
		
		std::vector<NodePT> seed_nodes;
		for (unsigned subgoal_idx = 0; subgoal_idx < _optimal_paths.size(); ++subgoal_idx) {
			if (!_in_seed[subgoal_idx] && _optimal_paths[subgoal_idx] != nullptr) {
				seed_nodes.push_back(_optimal_paths[subgoal_idx]);
			}
		}		
		
		std::vector<bool> R_G(index.size(), false);
		mark_atoms_in_path_to_subgoal(seed_nodes, R_G);
		
		unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
		if (_verbose) {
			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
			if (R_G_size) {
				LPT_INFO("cout", "Simulation - R_G:");
				std::cout << "\t\t";
				for (unsigned i = 0; i < R_G.size(); ++i) {
					if (R_G[i]) std::cout << index.to_atom(i) << ", ";
				}
				std::cout << std::endl;
			}
		}
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		
		return R_G;
	}
	
	std::vector<bool> extract_R_G_1() {
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		
		std::vector<NodePT> seed_nodes;
		for (unsigned subgoal_idx = 0; subgoal_idx < _optimal_paths.size(); ++subgoal_idx) {
			if (!_in_seed[subgoal_idx] && _optimal_paths[subgoal_idx] != nullptr) {
				seed_nodes.push_back(_optimal_paths[subgoal_idx]);
			}
		}		
		
		std::vector<bool> R_G(index.size(), false);
		mark_w1_atoms_in_path_to_subgoal(seed_nodes, R_G);
		
		unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
		if (_verbose) {
			LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
// 			if (R_G_size) {
// 				LPT_INFO("cout", "Simulation - R_G:");
// 				std::cout << "\t\t";
// 				for (unsigned i = 0; i < R_G.size(); ++i) {
// 					if (R_G[i]) std::cout << index.to_atom(i) << ", ";
// 				}
// 				std::cout << std::endl;
// 			}
		}
		_stats.relevant_atoms(R_G_size);
		_stats.r_type(2);
		
		return R_G;
	}	
	
	
	
// 	std::vector<AtomIdx> _compute_R(const StateT& seed) {
		
// 		_config._complete = false;
// 		
// 		float simt0 = aptk::time_used();
//  		run(seed);
// 		_stats.simulation();
// 		_stats.sim_add_time(aptk::time_used() - simt0);
// 		_stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
// 		_stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
// 		_stats.reachable_subgoals(_model.num_subgoals() - _unreached.size());
		
// 		std::vector<NodePT> w1_goal_reaching_nodes;
// 		std::vector<NodePT> w2_goal_reaching_nodes;
// 		std::vector<NodePT> wgt2_goal_reaching_nodes;

		
		/*
		LPT_INFO("cout", "Simulation - Number of novelty-1 nodes: " << _w1_nodes.size());
		LPT_INFO("cout", "Simulation - Number of novelty=1 nodes expanded in the simulation: " << _w1_nodes_expanded);
		LPT_INFO("cout", "Simulation - Number of novelty=2 nodes expanded in the simulation: " << _w2_nodes_expanded);
		LPT_INFO("cout", "Simulation - Number of novelty=1 nodes generated in the simulation: " << _w1_nodes_generated);
		LPT_INFO("cout", "Simulation - Number of novelty=2 nodes generated in the simulation: " << _w2_nodes_generated);
		LPT_INFO("cout", "Simulation - Number of novelty>2 nodes generated in the simulation: " << _w_gt2_nodes_generated);
		LPT_INFO("cout", "Simulation - Total number of generated nodes (incl. pruned): " << _generated);
		LPT_INFO("cout", "Simulation - Number of seed novelty-1 nodes: " << w1_seed_nodes.size());
		*/
		
// 		auto relevant_w2_nodes = compute_relevant_w2_nodes();
// 		LPT_INFO("cout", "Simulation - Number of relevant novelty-2 nodes: " << relevant_w2_nodes.size());
		
// 		auto su = compute_union(relevant_w2_nodes); // Order matters!
// 		auto hs = compute_hitting_set(relevant_w2_nodes);
		
// 		LPT_INFO("cout", "Simulation - union-based R (|R|=" << su.size() << ")");
// 		_print_atomset(su);
		
// 		LPT_INFO("cout", "Simulation - hitting-set-based-based R (|R|=" << hs.size() << ")");
// 		_print_atomset(hs);
		
		//std::vector<AtomIdx> relevant(hs.begin(), hs.end());
// 		std::vector<AtomIdx> relevant(su.begin(), su.end());
// 		std::sort(relevant.begin(), relevant.end());
// 		return relevant;
// 		return {};
// 	}
	
	bool run(const StateT& seed, unsigned max_width) {
		if (_verbose) LPT_INFO("cout", "Simulation - Starting IW Simulation");
		
		NodePT root = std::make_shared<NodeT>(seed, _generated++);
		mark_seed_subgoals(root);
		
		auto nov =_evaluator.evaluate(*root);
		assert(nov==1);
		update_novelty_counters_on_generation(nov);
		
// 		LPT_DEBUG("cout", "Simulation - Seed node: " << *root);
		
		assert(max_width <= 2); // The current swapping-queues method works only for up to width 2, but is trivial to generalize if necessary
		
		OpenListT open_w1, open_w2;
		OpenListT open_w1_next, open_w2_next; // The queues for the next depth level.
		
		open_w1.insert(root);
		
		while (true) {
			while (!open_w1.empty() || !open_w2.empty()) {
				NodePT current = open_w1.empty() ? open_w2.next() : open_w1.next();
				
				// Expand the node
				update_novelty_counters_on_expansion(current->_w);

				for (const auto& a : _model.applicable_actions(current->state)) {
					StateT s_a = _model.next( current->state, a );
					NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, _generated++);
					
					unsigned char novelty = _evaluator.evaluate(*successor);
					update_novelty_counters_on_generation(novelty);
					
					// LPT_INFO("cout", "Simulation - Node generated: " << *successor);
					
					if (process_node(successor)) {  // i.e. all subgoals have been reached before reaching the bound
						report("All subgoals reached");
						return true;
					}
					
					if (novelty <= max_width && novelty == 1) open_w1_next.insert(successor);
					else if (novelty <= max_width && novelty == 2) open_w2_next.insert(successor);
				}
				
			}
			// We've processed all nodes in the current depth level.
			open_w1.swap(open_w1_next);
			open_w2.swap(open_w2_next);
			
			if (open_w1.empty() && open_w2.empty()) break;
		}
		
		report("State space exhausted");
		return false;
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
	
	void report(const std::string& result) const {
		if (!_verbose) return;
		LPT_INFO("cout", "Simulation - Result: " << result);
		LPT_INFO("cout", "Simulation - Num reached subgoals: " << (_model.num_subgoals() - _unreached.size()) << " / " << _model.num_subgoals());
		LPT_INFO("cout", "Simulation - Expanded nodes with w=1 " << _w1_nodes_expanded);
		LPT_INFO("cout", "Simulation - Expanded nodes with w=2 " << _w2_nodes_expanded);		
		LPT_INFO("cout", "Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
		LPT_INFO("cout", "Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
		LPT_INFO("cout", "Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);
	}

protected:

	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node(NodePT& node) {
		if (_config._complete) return process_node_complete(node);
		
		const StateT& state = node->state;

		// We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
		// to check if the current node satisfies any of them - and if it does, we mark it appropriately.
		for (auto it = _unreached.begin(); it != _unreached.end(); ) {
			unsigned subgoal_idx = *it;

			if (_model.goal(state, subgoal_idx)) {
// 				node->satisfies_subgoal = true;
// 				_all_paths[subgoal_idx].push_back(node);
				if (!_optimal_paths[subgoal_idx]) _optimal_paths[subgoal_idx] = node;
				it = _unreached.erase(it);
			} else {
				++it;
			}
		}
		// As soon as all nodes have been processed, we return true so that we can stop the search
		return _unreached.empty();
	}
	
	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node_complete(NodePT& node) {
		const StateT& state = node->state;

		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (!_in_seed[i] && _model.goal(state, i)) {
// 				node->satisfies_subgoal = true;
				if (!_optimal_paths[i]) _optimal_paths[i] = node;
				_unreached.erase(i);
			}
		}
 		return _unreached.empty();
		//return false; // return false so we don't interrupt the processing
	}
	
	void mark_seed_subgoals(const NodePT& node) {
		std::vector<bool> _(_model.num_subgoals(), false);
		_in_seed.swap(_);
		_unreached.clear();
		for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
			if (_model.goal(node->state, i)) {
				_in_seed[i] = true;
			} else {
				_unreached.insert(i);
			}
		}
	}	

// public:
// 	const std::unordered_set<NodePT>& get_relevant_nodes() const { return _visited; }
};

} } // namespaces
