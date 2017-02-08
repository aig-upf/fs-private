
#pragma once

#include <iomanip>

#include <aptk2/tools/logging.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>
#include <lapkt/algorithms/generic_search.hxx>
#include <search/novelty/fs_novelty.hxx>
#include <search/drivers/sbfws/relevant_atomset.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/relevant_atomset.hxx>
#include <state.hxx>
// #include <lapkt/novelty/features.hxx>
#include <lapkt/components/open_lists.hxx>
// #include <boost/pool/pool_alloc.hpp>


namespace fs0 { namespace bfws {

template <typename StateT, typename ActionType>
class IWRunNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IWRunNode<StateT, ActionT>>;

	//! The state in this node
	StateT state;

	//! The (cached) feature valuation that corresponds to the state in this node.
// 	lapkt::novelty::FeatureValuation feature_valuation;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;
	
	//! Accummulated cost
	unsigned g;
	
	bool satisfies_subgoal; // Whether the node satisfies some subgoal
	
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
		_gen_order(gen_order)
	{}


	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "{@ = " << this;
		os << ", s = " << state ;
		os << ", g=" << g ;
// 		os << ", features= " << fs0::print::container(feature_valuation);
		os << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const IWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


//! This is the acceptor for an open list with width-based node pruning
//! - a node is pruned iff its novelty is higher than a given threshold.
template <typename NodeT, typename FeatureSetT, typename NoveltyEvaluatorT>
class IWRunAcceptor : public lapkt::QueueAcceptorI<NodeT> {
protected:
	//! The set of features used to compute the novelty
	const FeatureSetT& _features;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	NoveltyEvaluatorT* _evaluator;

public:
	IWRunAcceptor(const FeatureSetT& features, NoveltyEvaluatorT* evaluator) :
		_features(features),
		_evaluator(evaluator)
	{}

	~IWRunAcceptor() {
		delete _evaluator;
	}

	//! Returns false iff we want to prune this node during the search
	bool accept(NodeT& node) {
		unsigned novelty = 0;
		
		if (node.parent) {
			// Important: the novel-based computation works only when the parent has the same novelty type and thus goes against the same novelty tables!!!
			novelty = _evaluator->evaluate(_features.evaluate(node.state), _features.evaluate(node.parent->state));
		} else {
			novelty = _evaluator->evaluate(_features.evaluate(node.state));
		}
		
		return novelty < std::numeric_limits<unsigned>::max();
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
	

	using NodeOpenEvent = typename Base::NodeOpenEvent;
	using NodeExpansionEvent = typename Base::NodeExpansionEvent;
	using NodeCreationEvent = typename Base::NodeCreationEvent;
	
	struct Config {
		//! The bound on the simulation number of generated nodes
		unsigned _bound;
		
		//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
		bool _complete;
		
		//! Whether to take into account negative propositions or not
		bool _mark_negative;
		
		Config(unsigned bound, bool complete, bool mark_negative) :
			_bound(bound), _complete(complete), _mark_negative(mark_negative) {}
	};
	
protected:
	//! The simulation configuration
	const Config _config;

	//! _all_paths[i] contains all paths in the simulation that reach a node that satisfies goal atom 'i'.
	std::vector<std::vector<NodePT>> _all_paths;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	std::unordered_set<unsigned> _unreached;
	
	//! Contains the indexes of all those goal atoms that were already reached in the seed state
	std::vector<bool> _in_seed;

	//boost::fast_pool_allocator<NodeT> _allocator;
	
	//! Upon retrieval of the set of relevant atoms, this will contain all those nodes that are part
	//! of the path to some subgoal
	std::unordered_set<NodePT> _visited;
	
	//! The number of generated nodes so far
	unsigned long _generated;	

public:
	//! Factory method
	template <typename FeatureSetT, typename NoveltyEvaluatorT>
	static IWRun* build(const StateModel& model, const FeatureSetT& featureset, NoveltyEvaluatorT* evaluator, const IWRun::Config& config) {
		using AcceptorT = IWRunAcceptor<NodeT, FeatureSetT, NoveltyEvaluatorT>;
		auto acceptor = new AcceptorT(featureset, evaluator);
		return new IWRun(model, OpenListT(acceptor), config);
	}

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	IWRun(const StateModel& model, OpenListT&& open, const IWRun::Config& config) :
		Base(model, std::move(open), ClosedListT()),
		_config(config),
		_all_paths(model.num_subgoals()),
		_unreached(),
		_in_seed(model.num_subgoals(), false),
		_visited(),
		_generated(0)
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

	void run(const StateT& seed) {
		mark_seed_subgoals(seed);
		
		NodePT n = std::make_shared<NodeT>(seed, _generated);
		//NodePT n = std::allocate_shared<NodeT>(_allocator, seed);
		this->notify(NodeCreationEvent(*n));
		
		if (process_node(n)) return;
		
		this->_open.insert(n);

		while (!this->_open.empty()) {
			NodePT current = this->_open.next( );
			this->notify(NodeOpenEvent(*current));

			// close the node before the actual expansion so that children which are identical to 'current' get properly discarded.
			this->_closed.put(current);

			this->notify(NodeExpansionEvent(*current));

			for (const auto& a : this->_model.applicable_actions(current->state)) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current, _generated++ );
				//NodePT successor = std::allocate_shared<NodeT>( _allocator, std::move(s_a), a, current );

				if (this->_closed.check(successor)) continue; // The node has already been closed
				if (this->_open.contains(successor)) continue; // The node is already in the open list (and surely won't have a worse g-value, this being BrFS)

				if (process_node(successor)) return;

				this->notify(NodeCreationEvent(*successor));
				if (!this->_open.insert( successor )) {
					LPT_EDEBUG("search", std::setw(7) << "PRUNED: " << *successor);
				}
			}
		}
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
