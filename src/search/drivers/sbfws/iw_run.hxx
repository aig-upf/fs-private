
#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <iomanip>
#include <unordered_set>

#include <lapkt/tools/logging.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>
#include <lapkt/algorithms/generic_search.hxx>
#include <search/novelty/fs_novelty.hxx>
#include <search/drivers/sbfws/relevant_atomset.hxx>
#include "base.hxx"
#include <utils/printers/vector.hxx>
#include <utils/printers/actions.hxx>
#include <state.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <problem.hxx>


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
	
	bool satisfies_subgoal; // Whether the node satisfies some subgoal
	
	//! The novelty  of the state
	unsigned _w;
	
	//! NB(s)
	unsigned nb_s; 
	
	bool _evaluated;
	
	//!
	std::unordered_set<unsigned> _relevant_atoms;
	
	//! The indexes of the variables whose atoms form the set 1(s), which contains all atoms in 1(parent(s)) not deleted by the action that led to s, plus those 
	//! atoms in s with novelty 1.
	std::vector<unsigned> _nov1atom_idxs;
	
	//! Whether the path-novely of the node is one
	bool _path_novelty_is_1;
	
	//! The generation order, uniquely identifies the node
	unsigned long _gen_order;


	IWRunNode() = default;
	~IWRunNode() = default;
	IWRunNode(const IWRunNode&) = default;
	IWRunNode(IWRunNode&&) = delete;
	IWRunNode& operator=(const IWRunNode&) = delete;
	IWRunNode& operator=(IWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	IWRunNode(const StateT& s, unsigned long gen_order) : IWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	IWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, unsigned long gen_order) :
		state(std::move(_state)),
//		feature_valuation(0),
		action(_action),
		parent(_parent),
		g(parent ? parent->g+1 : 0),
		_w(std::numeric_limits<unsigned>::max()),
		_evaluated(false),
		_path_novelty_is_1(false),
		_gen_order(gen_order)
	{}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
// 		const Problem& problem = Problem::getInstance();
		os << "{@ = " << this;
		os << ", #=" << _gen_order ;
		os << ", s = " << state ;
		os << ", g=" << g ;
		os << ", w=" << (_evaluated ? (_w == std::numeric_limits<unsigned>::max() ? "INF" : std::to_string(_w)) : "?") ;
		
		os << ", act=" << action ;
// 		if (action < std::numeric_limits<unsigned>::max()) {
// 			os << ", act=" << *(problem.getGroundActions()[action]) ;
// 			os << fs0::print::full_action(*(problem.getGroundActions()[action]));
// 		} else {
// 			os << ", act=" << "NONE" ;
// 		}
		
		os << ", parent = " << (parent ? parent->_gen_order : -1) << "}";
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

	unsigned evaluate(NodeT& node) {
 		return evaluate_new(node);
		return evaluate_old(node);
	}
	
	//! Returns false iff we want to prune this node during the search
	unsigned evaluate_old(NodeT& node) {
		assert(!node._evaluated); // i.e. don't evaluate a node twice!
		node._evaluated = true;
		
		if (node.parent) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			node._w = _evaluator->evaluate(_features.evaluate(node.state), _features.evaluate(node.parent->state));
		} else {
			node._w = _evaluator->evaluate(_features.evaluate(node.state));
		}
		
// 		LPT_INFO("evaluated", "Evaluated: " << node << std::endl);
		
		if (node._w == 2) { // If the novelty is two, we want to store the set R_s of atoms that belong to a novel 2-tuple
			_evaluator->atoms_in_novel_tuple(node._relevant_atoms);
		}
		
		return node._w;
	}
	
	unsigned evaluate_new(NodeT& node) {
		assert(node._nov1atom_idxs.empty());
		assert(!node._path_novelty_is_1); // We still haven't determined whether the node has been reached through a nov-1 path
		
		assert(!node._evaluated); // i.e. don't evaluate a node twice!
		node._evaluated = true;		
		unsigned nov;
		auto valuation = _features.evaluate(node.state);
		
		if (!node.parent) { // We're dealing with the root node
			nov = _evaluator->evaluate_1(valuation, node._nov1atom_idxs);
			assert(nov == 1);
			node._path_novelty_is_1 = true;
			node.nb_s = 0;
			
			// NOW EVALUATE 1.5 novelty
			_evaluator->evaluate_piw(valuation);
		} else {
			auto parent_valuation = _features.evaluate(node.parent->state);
			std::vector<unsigned> new_atom_idxs;
			std::vector<unsigned> repeated_atom_idxs;
			
			analyze_new(valuation, parent_valuation, new_atom_idxs, repeated_atom_idxs);
			
			std::vector<unsigned> from_current, from_parent; // 'from_current' contains the indexes of those variables that contain novel atoms
			nov = _evaluator->evaluate_1(valuation, new_atom_idxs, from_current);
			node._path_novelty_is_1 = node.parent->_path_novelty_is_1 && (nov == 1);
			
			// Now add to nov1atoms those atoms in the parent state that had novelty 1 and have not been deleted by the action leading to this state
			const auto& parent_1s = node.parent->_nov1atom_idxs;
			
			std::set_intersection(parent_1s.begin(), parent_1s.end(), repeated_atom_idxs.begin(), repeated_atom_idxs.end(), std::back_inserter(from_parent));
			
			// 'from_parent' contains now the var-index of the undeleted atoms from 1(parent(s))
			
			// TODO Might want to remove this asserts at some point not to penalize the performance of the debug release too much
			assert(std::is_sorted(repeated_atom_idxs.begin(), repeated_atom_idxs.end()));
			assert(std::is_sorted(from_parent.begin(), from_parent.end()));
			assert(std::is_sorted(from_current.begin(), from_current.end()));			
			
			
			// SET 1(s) to its appropriate value
			std::set_union(from_parent.begin(), from_parent.end(), from_current.begin(), from_current.end(), std::back_inserter(node._nov1atom_idxs));
			/*
			if (node._path_novelty_is_1) {
				std::set_union(from_parent.begin(), from_parent.end(), from_current.begin(), from_current.end(), std::back_inserter(node._nov1atom_idxs));
			} else {
				node._nov1atom_idxs = std::move(from_parent);
			}
			*/
			
			std::vector<AtomIdx> B_of_s; // B(s)
			B_of_s = to_atom_indexes(node, node._nov1atom_idxs);
			
			
			
			// NOW EVALUATE 1.5 novelty

			node.nb_s = node.parent->nb_s;
				
			std::vector<bool> novelty_contributors;
			if (_evaluator->evaluate_piw(valuation, B_of_s, novelty_contributors)) {
				if (nov != 1) {
					nov = 2;
					
					// UPDATE NB(s) and prune, if necessary
					if (node.parent->_w == 1) {
						node.nb_s += 1;
						if (node.nb_s > 1) {
 							nov = 99; // Set novelty artificially high so that the node will get pruned.
//  							std::cout << "Pruned node because NB(s) > 1" << std::endl;
							
						}
					}
					
					
					if (nov == 2) {
						// UPDATE B(s)
						assert(node._nov1atom_idxs.size() == novelty_contributors.size());
						std::vector<unsigned> tmp;
						for (unsigned i1 = 0; i1 < novelty_contributors.size(); ++i1) {
							if (novelty_contributors[i1]) {
								tmp.push_back(node._nov1atom_idxs[i1]);
							}
						}
						
	// 					std::cout << "Pruned 1(s) from " << node._nov1atom_idxs.size() << " to " << tmp.size()  << " (out of a max. of " << node.state.numAtoms() << " atoms in a state) "<< std::endl;
						node._nov1atom_idxs = tmp;
	// 					std::cout << "Simulation node - NB(s)=" << node.nb_s << std::endl;
					}
				}
			}
			
		}
		
		
		node._w = nov;
		return nov;
	}
	
	std::vector<AtomIdx> to_atom_indexes(const NodeT& node, const std::vector<unsigned>& variables) {
		std::vector<AtomIdx> special;
		const AtomIndex& atomidx = Problem::getInstance().get_tuple_index();
		for (unsigned var:variables) {
			auto val = node.state.getValue(var);
			if (val == 0) continue;
			special.push_back(atomidx.to_index(var, val));
		}
		return special;
	}
	

	//! Compute a vector with the indexes of those elements in a given valuation that are novel wrt a "parent" valuation.
	template <typename FeatureValueT>
	void analyze_new(const std::vector<FeatureValueT>& current, const std::vector<FeatureValueT>& parent, std::vector<unsigned>& new_atoms, std::vector<unsigned>& repeated_atoms) {
		assert(current.size() == parent.size());
		assert(new_atoms.empty());
		assert(repeated_atoms.empty());
		for (unsigned i = 0; i < current.size(); ++i) {
			if (current[i] != parent[i]) {
				new_atoms.push_back(i);
			} else {
				repeated_atoms.push_back(i);
			}
		}
	}
		
	
	std::vector<bool> reached_atoms() const {
		std::vector<bool> atoms;
		_evaluator->mark_atoms_in_novelty1_table(atoms);
		return atoms;
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
		  typename FeatureSetT,
          typename OpenListT = lapkt::SearchableQueue<NodeT>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class IWRun : public lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>
{
public:
	using Base = lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	using ActionT = typename StateModel::ActionType;
	using StateT = typename StateModel::StateT;
	using PlanT = typename Base::PlanT;
	using NodePT = typename Base::NodePT;
	using SimEvaluatorT = SimulationEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
	

	using NodeOpenEvent = typename Base::NodeOpenEvent;
	using NodeExpansionEvent = typename Base::NodeExpansionEvent;
	using NodeCreationEvent = typename Base::NodeCreationEvent;
	
	struct Config {
		//! The bound on the simulation number of generated nodes
		int _bound;
		
		//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
		bool _complete;
		
		//! Whether to take into account negative propositions or not
		bool _mark_negative;
		
		//!
		unsigned _max_width;
		
		bool _use_goal_directed_info;
		
		Config(int bound, bool complete, bool mark_negative, unsigned max_width, bool goal_directed_info) :
			_bound(bound), _complete(complete), _mark_negative(mark_negative), _max_width(max_width), _use_goal_directed_info(goal_directed_info) {}
		
	};
	
protected:
	//! The simulation configuration
	Config _config;

	//! _all_paths[i] contains all paths in the simulation that reach a node that satisfies goal atom 'i'.
//  	std::vector<std::vector<NodePT>> _all_paths;

 	std::vector<NodePT> _optimal_paths;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	// TODO REMOVE
	std::unordered_set<unsigned> _unreached;
	
	//! Contains the indexes of all those goal atoms that were already reached in the seed state
	std::vector<bool> _in_seed;

	//boost::fast_pool_allocator<NodeT> _allocator;
	
	//! Upon retrieval of the set of relevant atoms, this will contain all those nodes that are part
	//! of the path to some subgoal
// 	std::unordered_set<NodePT> _visited;
	
	//! A single novelty evaluator will be in charge of evaluating all nodes
	SimEvaluatorT _evaluator;
	
	//! All those nodes with width 1 on the first stage of the simulation
	std::vector<NodePT> _w1_nodes;
	
	//! Some node counts
	unsigned long _generated;
	unsigned long _w1_nodes_expanded;
	unsigned long _w2_nodes_expanded;
	unsigned long _w1_nodes_generated;
	unsigned long _w2_nodes_generated;
	unsigned long _w_gt2_nodes_generated;

public:

	//! Constructor
	IWRun(const StateModel& model, const FeatureSetT& featureset, const IWRun::Config& config) :
		Base(model, OpenListT(), ClosedListT()),
		_config(config),
// 		_all_paths(model.num_subgoals()),
		_optimal_paths(model.num_subgoals()),
		_unreached(),
		_in_seed(model.num_subgoals(), false),
// 		_visited(),
		_evaluator(featureset, create_novelty_evaluator<NoveltyEvaluatorT>(model.getTask(), SBFWSConfig::NoveltyEvaluatorType::Adaptive, _config._max_width, false)),
		_w1_nodes(),
		_generated(0),
		_w1_nodes_expanded(0),
		_w2_nodes_expanded(0),
		_w1_nodes_generated(0),
		_w2_nodes_generated(0),		
		_w_gt2_nodes_generated(0)
	{
	}

	~IWRun() = default;

	// Disallow copy, but allow move
	IWRun(const IWRun&) = delete;
	IWRun(IWRun&&) = default;
	IWRun& operator=(const IWRun&) = delete;
	IWRun& operator=(IWRun&&) = default;

	bool search(const StateT& s, PlanT& solution) override {
		throw std::runtime_error("Shouldn't be invoking this");
	}
	
	
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
				for (unsigned var = 0; var < state.numAtoms(); ++var) {
					if (state.getValue(var) == 0) continue; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
					AtomIdx atom = index.to_index(var, state.getValue(var));
					atoms[atom] = true;
				}
				
				node = node->parent;
			}			
		}
	}
	
	
	std::vector<bool> compute_R(const StateT& seed) {
		if (_config._use_goal_directed_info) {
			return compute_goal_directed_R(seed);
		} else {
			return compute_R_IW1(seed);
		}
	}
	
	std::vector<bool> compute_R_IW1(const StateT& seed) {
		LPT_INFO("cout", "IW Simulation - Computing blind R");
		_config._max_width = 1;
		_config._bound = -1; // No bound
		std::vector<NodePT> seed_nodes;
		_compute_R(seed, seed_nodes);

		LPT_INFO("cout", "IW Simulation - Number of seed nodes: " << seed_nodes.size());
		std::vector<bool> rel_blind = _evaluator.reached_atoms();
		LPT_INFO("cout", "IW Simulation - Blind |R|         = " << std::count(rel_blind.begin(), rel_blind.end(), true));
		return rel_blind;
	}
	
	std::vector<bool> compute_goal_directed_R(const StateT& seed) {
		LPT_INFO("cout", "IW Simulation - Computing goal-directed R");
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		_config._max_width = 2;
		_config._bound = -1; // No bound
		std::vector<NodePT> seed_nodes;
		_compute_R(seed, seed_nodes);

		LPT_INFO("cout", "IW Simulation - Number of seed nodes: " << seed_nodes.size());
		if (!_unreached.empty()) {
			LPT_INFO("cout", "Some subgoals not reached during the simulation. ABORTING");
			for (unsigned x:_unreached) {
				LPT_INFO("cout", "\t Unreached subgoal idx: " << x);
			}
			exit(1);
		}
		
		std::vector<bool> rel_goal_directed(index.size(), false);
		mark_atoms_in_path_to_subgoal(seed_nodes, rel_goal_directed);
		LPT_INFO("cout", "IW Simulation - Goal-directed |R| = " << std::count(rel_goal_directed.begin(), rel_goal_directed.end(), true));
		return rel_goal_directed;		
	}	
	
	
	std::vector<AtomIdx> _compute_R(const StateT& seed, std::vector<NodePT>& seed_nodes) {
		
		_config._complete = false;
		
 		_run(seed);
		
		LPT_INFO("cout", "IW Simulation - Num unreached subgoals: " << _unreached.size() << " / " << this->_model.num_subgoals());
		
// 		std::vector<NodePT> w1_goal_reaching_nodes;
// 		std::vector<NodePT> w2_goal_reaching_nodes;
// 		std::vector<NodePT> wgt2_goal_reaching_nodes;
		
		
		/*
		for (unsigned subgoal_idx = 0; subgoal_idx < _all_paths.size(); ++subgoal_idx) {
			const std::vector<NodePT>& paths = _all_paths[subgoal_idx];
			assert(_in_seed[subgoal_idx] || !paths.empty());
			seed_nodes.insert(seed_nodes.end(), paths.begin(), paths.end());
		}
		*/

		for (unsigned subgoal_idx = 0; subgoal_idx < _optimal_paths.size(); ++subgoal_idx) {
			if (!_in_seed[subgoal_idx] && _optimal_paths[subgoal_idx] != nullptr) {
				seed_nodes.push_back(_optimal_paths[subgoal_idx]);
			}
		}
		
		/*
		LPT_INFO("cout", "IW Simulation - Number of novelty-1 nodes: " << _w1_nodes.size());
		LPT_INFO("cout", "IW Simulation - Number of novelty=1 nodes expanded in the simulation: " << _w1_nodes_expanded);
		LPT_INFO("cout", "IW Simulation - Number of novelty=2 nodes expanded in the simulation: " << _w2_nodes_expanded);
		LPT_INFO("cout", "IW Simulation - Number of novelty=1 nodes generated in the simulation: " << _w1_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Number of novelty=2 nodes generated in the simulation: " << _w2_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Number of novelty>2 nodes generated in the simulation: " << _w_gt2_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Total number of generated nodes (incl. pruned): " << _generated);
		LPT_INFO("cout", "IW Simulation - Number of seed novelty-1 nodes: " << w1_seed_nodes.size());
		*/
		
// 		auto relevant_w2_nodes = compute_relevant_w2_nodes();
// 		LPT_INFO("cout", "IW Simulation - Number of relevant novelty-2 nodes: " << relevant_w2_nodes.size());
		
// 		auto su = compute_union(relevant_w2_nodes); // Order matters!
// 		auto hs = compute_hitting_set(relevant_w2_nodes);
		
// 		LPT_INFO("cout", "IW Simulation - union-based R (|R|=" << su.size() << ")");
// 		_print_atomset(su);
		
// 		LPT_INFO("cout", "IW Simulation - hitting-set-based-based R (|R|=" << hs.size() << ")");
// 		_print_atomset(hs);
		
		//std::vector<AtomIdx> relevant(hs.begin(), hs.end());
// 		std::vector<AtomIdx> relevant(su.begin(), su.end());
// 		std::sort(relevant.begin(), relevant.end());
// 		return relevant;
		return {};
	}
	
	bool _run(const StateT& seed) {
		NodePT n = std::make_shared<NodeT>(seed, _generated++);
		mark_seed_subgoals(n);
		
		auto nov =_evaluator.evaluate(*n);
		_unused(nov);
		assert(nov==1);
// 		LPT_DEBUG("cout", "IW Simulation - Seed node: " << *n);

		this->_open.insert(n);

		unsigned accepted = 1; // (the root node counts as accepted)
		
		while (!this->_open.empty()) {
			NodePT current = this->_open.next( );

			// close the node before the actual expansion so that children which are identical to 'current' get properly discarded.
			this->_closed.put(current);
			
			if (current->_w == 1) ++_w1_nodes_expanded;
			else if (current->_w == 2) ++_w2_nodes_expanded;

			for (const auto& a : this->_model.applicable_actions(current->state)) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current, _generated++ );
				
				if (this->_closed.check(successor)) continue; // The node has already been closed
				if (this->_open.contains(successor)) continue; // The node is already in the open list (and surely won't have a worse g-value, this being BrFS)

				
				unsigned novelty = _evaluator.evaluate(*successor);
				if (novelty == 1) {
					_w1_nodes.push_back(successor);
				}
				
//   				LPT_INFO("cout", "IW Simulation - Node generated: " << *successor);
				
				if (process_node(successor)) {  // i.e. all subgoals have been reached before reaching the bound
					LPT_INFO("cout", "IW Simulation - All subgoals reached after processing " << accepted << " nodes");
					LPT_INFO("cout", "IW Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
					LPT_INFO("cout", "IW Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
					LPT_INFO("cout", "IW Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);					
					return true;
				}
				
				if (novelty <= _config._max_width) {
					this->_open.insert(successor);
					accepted++;
					
					assert(novelty == 1 || novelty == 2);
					
					if (novelty==1) ++_w1_nodes_generated;
					else  ++_w2_nodes_generated;
				} else {
					++_w_gt2_nodes_generated;
				}
				
				if (_config._bound > 0 && accepted >= (unsigned) _config._bound) {
 					LPT_INFO("cout", "IW Simulation - Bound reached: " << accepted << " nodes processed");
					return false;
				}
			}
		}
		
		LPT_INFO("cout", "IW Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);
		
 		LPT_INFO("cout", "IW Simulation - State space exhausted after exploring " << accepted << " nodes");
// 		LPT_INFO("cout", "IW Simulation - # unreached subgoals: " << _unreached.size());
		return false; // Or the state space is exhausted before either reaching all subgoals or reaching the bound
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

			if (this->_model.goal(state, subgoal_idx)) {
				node->satisfies_subgoal = true;
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

		for (unsigned i = 0; i < this->_model.num_subgoals(); ++i) {
			if (!_in_seed[i] && this->_model.goal(state, i)) {
				node->satisfies_subgoal = true;
				if (!_optimal_paths[i]) _optimal_paths[i] = node;
				_unreached.erase(i);
			}
		}
 		return _unreached.empty();
		//return false; // return false so we don't interrupt the processing
	}
	
	void mark_seed_subgoals(const NodePT& node) {
		for (unsigned i = 0; i < this->_model.num_subgoals(); ++i) {
			if (this->_model.goal(node->state, i)) {
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
