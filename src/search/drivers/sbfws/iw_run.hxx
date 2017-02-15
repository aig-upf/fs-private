
#pragma once

#include <iomanip>
#include <unordered_set>

#include <aptk2/tools/logging.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>
#include <lapkt/algorithms/generic_search.hxx>
#include <search/novelty/fs_novelty.hxx>
#include <search/drivers/sbfws/relevant_atomset.hxx>
#include "base.hxx"
#include <utils/printers/vector.hxx>
#include <utils/printers/relevant_atomset.hxx>
#include <utils/printers/actions.hxx>
#include <state.hxx>
// #include <lapkt/novelty/features.hxx>
#include <lapkt/components/open_lists.hxx>
#include <problem.hxx>
// #include <boost/pool/pool_alloc.hpp>


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
	
	bool _evaluated;
	
	//!
	std::unordered_set<unsigned> _relevant_atoms;
	
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
		_gen_order(gen_order)
	{}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		const Problem& problem = Problem::getInstance();
		os << "{@ = " << this;
		os << ", #=" << _gen_order ;
		os << ", s = " << state ;
		os << ", g=" << g ;
		os << ", w=" << (_evaluated ? (_w == std::numeric_limits<unsigned>::max() ? "INF" : std::to_string(_w)) : "?") ;
		
		if (action < std::numeric_limits<unsigned>::max()) {
			os << ", act=" << *(problem.getGroundActions()[action]) ;
// 			os << fs0::print::full_action(*(problem.getGroundActions()[action]));
		} else {
			os << ", act=" << "NONE" ;
		}
		
		os << ", parent = " << parent << "}";
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
		
		Config(int bound, bool complete, bool mark_negative) :
			_bound(bound), _complete(complete), _mark_negative(mark_negative), _max_width(2) {}
		
		
	};
	
protected:
	//! The simulation configuration
	Config _config;

	//! _all_paths[i] contains all paths in the simulation that reach a node that satisfies goal atom 'i'.
	std::vector<std::vector<NodePT>> _all_paths;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	// TODO REMOVE
	std::unordered_set<unsigned> _unreached;
	
	//! Contains the indexes of all those goal atoms that were already reached in the seed state
	std::vector<bool> _in_seed;

	//boost::fast_pool_allocator<NodeT> _allocator;
	
	//! Upon retrieval of the set of relevant atoms, this will contain all those nodes that are part
	//! of the path to some subgoal
	std::unordered_set<NodePT> _visited;
	
	//! A single novelty evaluator will be in charge of evaluating all nodes
	SimEvaluatorT _evaluator;
	
	//! All those nodes with width 1 on the first stage of the simulation
	std::vector<NodePT> _w1_nodes;
	
	//! The number of generated nodes so far
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
		_all_paths(model.num_subgoals()),
		_unreached(),
		_in_seed(model.num_subgoals(), false),
		_visited(),
		_evaluator(featureset, create_novelty_evaluator<NoveltyEvaluatorT>(model.getTask(), SBFWSConfig::NoveltyEvaluatorType::Adaptive, 1, true)),
		_w1_nodes(),
		_generated(0),
		_w1_nodes_expanded(0),
		_w2_nodes_expanded(0),
		_w1_nodes_generated(0),
		_w2_nodes_generated(0),		
		_w_gt2_nodes_generated(0)
	{

		for (unsigned i = 0; i < model.num_subgoals(); ++i) _unreached.insert(i); // Initially all goal atoms assumed to be unreached
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
	
	std::unordered_set<NodePT> compute_relevant_w2_nodes() const {
		std::unordered_set<NodePT> all_visited;
		std::unordered_set<NodePT> w2_nodes;
		for (NodePT node:_w1_nodes) {
			process_w1_node(node, w2_nodes, all_visited);
		}
		return w2_nodes;
	}
	
	void process_w1_node(NodePT node, std::unordered_set<NodePT>& w2_nodes, std::unordered_set<NodePT>& all_visited) const {
		// Traverse from the solution node to the root node
		
		NodePT root = node;
		// We ignore s0
		while (node->has_parent()) {
			// If the node has already been processed, no need to do it again, nor to process the parents,
			// which will necessarily also have been processed.
			auto res = all_visited.insert(node);
			if (!res.second) break;
			
			if (node->_w == 2) {
				w2_nodes.insert(node);
			}
			node = node->parent;
		}
	}
	
	AtomIdx select_atom(const std::unordered_set<NodePT>& nodes) const {
		std::unordered_map<AtomIdx, unsigned> counts;
		
		for (const auto& node:nodes) {
			for (const AtomIdx atom:node->_relevant_atoms) {
				counts[atom]++;
			}
		}
		
// 		for (const auto& c:counts) LPT_INFO("cout", "Atom " << c.first << " count: " << c.second);
		
		using T = typename std::unordered_map<AtomIdx, unsigned>::value_type;
		return std::max_element(counts.begin(), counts.end(), [](T a, T b){ return a.second < b.second; })->first;
	}
	
	// TODO Optimize this very inefficient prototype
	std::unordered_set<AtomIdx> compute_hitting_set(std::unordered_set<NodePT>& nodes) const {
		std::unordered_set<AtomIdx> hs;
		while (!nodes.empty()) {
			AtomIdx selected = select_atom(nodes);
			
// 			LPT_INFO("cout", "IW Simulation - Selected atom: " << selected << ": " << index.to_atom(selected));
			
			for (auto it = nodes.begin(); it != nodes.end();) {
				const std::unordered_set<unsigned>& rset = (*it)->_relevant_atoms;
				if (rset.find(selected) != rset.end()) {
					it = nodes.erase(it);
				} else {
					++it;
				}
			}
			hs.insert(selected);
		}
		
		return hs;
		
	}
	
	std::unordered_set<AtomIdx> compute_union(std::unordered_set<NodePT>& nodes) const {
		std::unordered_set<AtomIdx> set_union;
		for (const auto& node:nodes) {
			set_union.insert(node->_relevant_atoms.begin(), node->_relevant_atoms.end());
		}
		return set_union;
	}
	
	void _print_atomset(std::unordered_set<AtomIdx>& atoms) const {
		const AtomIndex& index = this->_model.getTask().get_tuple_index();
		for (AtomIdx atom:atoms) {
			LPT_INFO("cout", "IW Simulation - \t" << index.to_atom(atom));
		}		
	}
	
	void run(const StateT& seed) {
		throw std::runtime_error("This no longer works :-)");
	}
	
	std::vector<bool> compute_R_IW1(const StateT& seed) {
		_config._max_width = 1;
		_config._bound = -1; // No bound
		compute_R(seed);
		auto rset = _evaluator.reached_atoms();
		LPT_INFO("cout", "IW Simulation - |R_{IW(1)}| = " << std::count(rset.begin(), rset.end(), true)); // TODO REMOVE THIS, IT'S EXPENSIVE
		return rset;
	}

	std::vector<bool> compute_R_union_Rs(const StateT& seed) {
		_config._max_width = 2;
// 		_config._bound = -1; // No bound
		auto atoms = compute_R(seed);
		
		const AtomIndex& index = Problem::getInstance().get_tuple_index();
		std::vector<bool> res(index.size(), false);
		for (AtomIdx atom:atoms) {
			res[atom] = true;
		}
		return res;
	}

	
	
	std::vector<AtomIdx> compute_R(const StateT& seed) {
		
		_config._complete = false;
		
		bool all_reached_before_bound = _run(seed);
		
		std::vector<NodePT> w1_seed_nodes;
		if (all_reached_before_bound) {
			for (const auto& n:_w1_nodes) {
				if (n->satisfies_subgoal) w1_seed_nodes.push_back(n);
			}
		} else {
			w1_seed_nodes = _w1_nodes;
		}
		
		LPT_INFO("cout", "IW Simulation - All subgoals reached before bound? " << all_reached_before_bound);
		LPT_INFO("cout", "IW Simulation - Num unreached subgoals: " << _unreached.size() << " / " << this->_model.num_subgoals());
		LPT_INFO("cout", "IW Simulation - Number of novelty-1 nodes: " << _w1_nodes.size());
		LPT_INFO("cout", "IW Simulation - Number of novelty=1 nodes expanded in the simulation: " << _w1_nodes_expanded);
		LPT_INFO("cout", "IW Simulation - Number of novelty=2 nodes expanded in the simulation: " << _w2_nodes_expanded);
		
		LPT_INFO("cout", "IW Simulation - Number of novelty=1 nodes generated in the simulation: " << _w1_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Number of novelty=2 nodes generated in the simulation: " << _w2_nodes_generated);
		LPT_INFO("cout", "IW Simulation - Number of novelty>2 nodes generated in the simulation: " << _w_gt2_nodes_generated);
		
		
		LPT_INFO("cout", "IW Simulation - Total number of generated nodes (incl. pruned): " << _generated);
		
		
		LPT_INFO("cout", "IW Simulation - Number of seed novelty-1 nodes: " << w1_seed_nodes.size());
		
		auto relevant_w2_nodes = compute_relevant_w2_nodes();
		LPT_INFO("cout", "IW Simulation - Number of relevant novelty-2 nodes: " << relevant_w2_nodes.size());
		
		auto su = compute_union(relevant_w2_nodes); // Order matters!
		auto hs = compute_hitting_set(relevant_w2_nodes);
		
		LPT_INFO("cout", "IW Simulation - union-based R (|R|=" << su.size() << ")");
// 		_print_atomset(su);
		
		
		LPT_INFO("cout", "IW Simulation - hitting-set-based-based R (|R|=" << hs.size() << ")");
// 		_print_atomset(hs);
		
		//std::vector<AtomIdx> relevant(hs.begin(), hs.end());
		std::vector<AtomIdx> relevant(su.begin(), su.end());
		std::sort(relevant.begin(), relevant.end());
		return relevant;
	}
	
	bool _run(const StateT& seed) {
		mark_seed_subgoals(seed);
		
		NodePT n = std::make_shared<NodeT>(seed, _generated++);
		//NodePT n = std::allocate_shared<NodeT>(_allocator, seed);
// 		this->notify(NodeCreationEvent(*n));
		
		
		auto nov =_evaluator.evaluate(*n);
		_unused(nov);
		assert(nov==1);
		LPT_INFO("cout", "IW Simulation - Seed node: " << *n);

		
// 		if (process_node(n)) return;
		
		this->_open.insert(n);

		unsigned accepted = 1; // (the root node counts as accepted)
		
		while (!this->_open.empty()) {
			NodePT current = this->_open.next( );
// 			this->notify(NodeOpenEvent(*current));

			// close the node before the actual expansion so that children which are identical to 'current' get properly discarded.
			this->_closed.put(current);

// 			this->notify(NodeExpansionEvent(*current));
			
			if (current->_w == 1) ++_w1_nodes_expanded;
			else if (current->_w == 2) ++_w2_nodes_expanded;

			for (const auto& a : this->_model.applicable_actions(current->state)) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current, _generated++ );
				//NodePT successor = std::allocate_shared<NodeT>( _allocator, std::move(s_a), a, current );

// 				LPT_INFO("cout", "IW Simulation - Node generated0: " << *successor);
				
				if (this->_closed.check(successor)) continue; // The node has already been closed
				if (this->_open.contains(successor)) continue; // The node is already in the open list (and surely won't have a worse g-value, this being BrFS)

// 				if (process_node(successor)) return;

// 				this->notify(NodeCreationEvent(*successor));
				
				unsigned novelty = _evaluator.evaluate(*successor);
				if (novelty == 1) {
					_w1_nodes.push_back(successor);
				}
				
// 				LPT_INFO("cout", "IW Simulation - Node generated: " << *successor);
				
				if (process_node(successor)) {  // i.e. all subgoals have been reached before reaching the bound
					LPT_INFO("cout", "IW Simulation - All subgoals reached after processing " << accepted << " nodes");
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
				
				if (_config._bound > 0 && accepted >= _config._bound) {
					LPT_INFO("cout", "IW Simulation - Bound reached: " << accepted << " nodes processed");
					return false;
				}
			}
		}
		LPT_INFO("cout", "IW Simulation - State space exhausted after exploring " << accepted << " nodes");
		LPT_INFO("cout", "IW Simulation - # unreached subgoals: " << _unreached.size());
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
				_all_paths[subgoal_idx].push_back(node);
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
				_all_paths[i].push_back(node);
			}
		}
		return false;
	}
	
	bool mark_seed_subgoals(const StateT& seed) {
		for (unsigned i = 0; i < this->_model.num_subgoals(); ++i) {
			if (this->_model.goal(seed, i)) {
				_unreached.erase(i);
				_in_seed[i] = true;
			}
		}
		return false;
	}	

public:
	//! Retrieve the set of atoms which are relevant to reach at least one of the subgoals
	//! Additionally, leaves in the class attribute '_visited' pointers to all those nodes which
	//! are on at least one of the paths from the seed state to one of the nodes that satisfies a subgoal.
	RelevantAtomSet retrieve_relevant_atoms(const StateT& seed, unsigned& reachable) {
		const AtomIndex& atomidx = this->_model.getTask().get_tuple_index();
		RelevantAtomSet atomset(&atomidx);

		// atomset.mark(seed, RelevantAtomSet::STATUS::UNREACHED); // This is not necessary, since all these atoms will be made true by the "root" state of the simulation

		_visited.clear();

		// Iterate through all the subgoals that have been reached, and rebuild the path from the seed state to reach them
		// adding all atoms encountered in the path to the RelevantAtomSet as "relevant but unreached"
		for (unsigned subgoal_idx = 0; subgoal_idx < _all_paths.size(); ++subgoal_idx) {
			const std::vector<NodePT>& paths = _all_paths[subgoal_idx];
			
			if (_in_seed[subgoal_idx] || !paths.empty()) {
				++reachable;
			}
			
			for (const NodePT& node:paths) {
				process_path_node(node, atomset);
			}
		}

		return atomset;
	}
	
	void process_path_node(NodePT node, RelevantAtomSet& atomset) {
		// Traverse from the solution node to the root node, adding all atoms on the way
		// if (node->has_parent()) node = node->parent; // (Don't) skip the last node
		while (node->has_parent()) {
			// If the node has already been processed, no need to do it again, nor to process the parents,
			// which will necessarily also have been processed.
			if (_visited.find(node) != _visited.end()) break;
			
			// Mark all the atoms in the state as "yet to be reached"
			atomset.mark(node->state, &(node->parent->state), RelevantAtomSet::STATUS::UNREACHED, _config._mark_negative, false);
			_visited.insert(node);
			node = node->parent;
		}
		
		_visited.insert(node); // Insert the root anyway to mark it as a relevant node
	}
	
	
	const std::unordered_set<NodePT>& get_relevant_nodes() const { return _visited; }
};

} } // namespaces
