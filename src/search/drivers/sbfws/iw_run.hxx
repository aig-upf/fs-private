
#pragma once

#include <iomanip>

#include <aptk2/tools/logging.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>
#include <lapkt/algorithms/generic_search.hxx>
#include <search/drivers/bfws/iw_novelty_evaluator.hxx>
#include <problem_info.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/relevant_atomset.hxx>
#include <utils/atom_index.hxx>
#include <state.hxx>
#include <lapkt/novelty/features.hxx>
#include <lapkt/components/open_lists.hxx>
// #include <boost/pool/pool_alloc.hpp>


namespace fs0 { namespace bfws {

//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:
	enum class STATUS : unsigned char {IRRELEVANT, UNREACHED, REACHED};

	//! A RelevantAtomSet is always constructed with all atoms being marked as IRRELEVANT
	RelevantAtomSet(const AtomIndex* atomidx) :
		_atomidx(atomidx), _num_reached(0), _num_unreached(0), _status(atomidx ? atomidx->size() : 0, STATUS::IRRELEVANT)
	{}

	~RelevantAtomSet() = default;
	RelevantAtomSet(const RelevantAtomSet&) = default;
	RelevantAtomSet(RelevantAtomSet&&) = default;
	RelevantAtomSet& operator=(const RelevantAtomSet&) = default;
	RelevantAtomSet& operator=(RelevantAtomSet&&) = default;

	//! Marks all the atoms in the state with the given 'status'.
	//! If 'mark_negative_propositions' is false, predicative atoms of the form X=false are ignored
	//! If 'only_if_relevant' is true, only those atoms that were not deemed _irrelevant_ (i.e. their status was either reached or unreached)
	//! are marked
	void mark(const State& state, const State* parent, STATUS status, bool mark_negative_propositions, bool only_if_relevant) {
		assert(_atomidx);
		const ProblemInfo& info = ProblemInfo::getInstance();
		for (VariableIdx var = 0; var < state.numAtoms(); ++var) {
			ObjectIdx val = state.getValue(var);
			if (!mark_negative_propositions && info.isPredicativeVariable(var) && val==0) continue; // We don't want to mark negative propositions
			if (parent && (val == parent->getValue(var))) continue; // If a parent was provided, we check that the value is new wrt the parent
			mark(_atomidx->to_index(var, val), status, only_if_relevant);
		}
	}

	//! Marks the atom with given index with the given status.
	//! If 'only_if_relevant' is true, then only marks the atom if it was previously
	//! marked as relevant (i.e. its status was _not_ STATUS::IRRELEVANT).
	void mark(AtomIdx idx, STATUS status, bool only_if_relevant) {
		assert(status==STATUS::REACHED || status==STATUS::UNREACHED);
		auto& st = _status[idx];
		if (only_if_relevant && (st == STATUS::IRRELEVANT)) return;

		if (st != status) {
			if (status==STATUS::REACHED) ++_num_reached;
			else if (status==STATUS::UNREACHED) ++_num_unreached;
			
			if (st==STATUS::REACHED) --_num_reached; // The old status was reached, and will not be anymore, so we decrease the counter.
			else if (st==STATUS::UNREACHED) --_num_unreached;
			
			st = status;
		}
	}

	unsigned num_reached() const { return _num_reached; }
	unsigned num_unreached() const { return _num_unreached; }
	
	bool valid() const { return _atomidx != nullptr; }

	friend class print::relevant_atomset;

protected:
	//! A reference to the global atom index
	const AtomIndex* _atomidx;

	//! The total number of reached / unreached atoms
	unsigned _num_reached;
	unsigned _num_unreached;

	//! The status of each atom (indexed by its atom index)
	std::vector<STATUS> _status;
};



template <typename StateT, typename ActionType>
class IWRunNode {
public:
	using ActionT = ActionType;
	using PT = std::shared_ptr<IWRunNode<StateT, ActionT>>;

	//! The state in this node
	StateT state;

	//! The (cached) feature valuation that corresponds to the state in this node.
	lapkt::novelty::FeatureValuation feature_valuation;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;
	
	//! Accummulated cost
	unsigned g;


	IWRunNode() = default;
	~IWRunNode() = default;
	IWRunNode(const IWRunNode&) = default;
	IWRunNode(IWRunNode&&) = delete;
	IWRunNode& operator=(const IWRunNode&) = delete;
	IWRunNode& operator=(IWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	IWRunNode(const StateT& s, unsigned long gen_order = 0) : IWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

	//! Constructor with move of the state (cheaper)
	IWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, unsigned long gen_order = 0) :
		state(std::move(_state)),
		feature_valuation(0),
		action(_action),
		parent(_parent),
		g(parent ? parent->g+1 : 0)
	{}

	//! The novelty type (for the IWRun node, will always be 0)
	unsigned type() const { return 0; }

	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "{@ = " << this;
		os << ", s = " << state ;
		os << ", g=" << g ;
		os << ", features= " << fs0::print::container(feature_valuation);
		os << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const IWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


//! This is the acceptor for an open list with width-based node pruning
//! - a node is pruned iff its novelty is higher than a given threshold.
template <typename NodeT>
class IWRunAcceptor : public lapkt::QueueAcceptorI<NodeT> {
protected:
	//! The set of features used to compute the novelty
	const lapkt::novelty::FeatureSet<State>& _features;

	//! A single novelty evaluator will be in charge of evaluating all nodes
	IWNoveltyEvaluator _novelty_evaluator;

public:
	IWRunAcceptor(const lapkt::novelty::FeatureSet<State>& features, const IWNoveltyEvaluator& novelty_evaluator) :
		_features(features),
		_novelty_evaluator(novelty_evaluator) // Copy the evaluator
	{}

	~IWRunAcceptor() = default;

	//! Returns false iff we want to prune this node during the search
	bool accept(NodeT& node) {
		assert(node.feature_valuation.empty()); // We should not be computing the feature valuation twice!
		node.feature_valuation = _features.evaluate(node.state);
		unsigned novelty = _novelty_evaluator.evaluate(node);
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


	//! Factory method
	static IWRun* build(const StateModel& model, const lapkt::novelty::FeatureSet<State>& featureset, const IWNoveltyEvaluator& novelty_evaluator, bool mark_negative_propositions) {
		auto acceptor = new IWRunAcceptor<NodeT>(featureset, novelty_evaluator);
		return new IWRun(model, OpenListT(acceptor), false, mark_negative_propositions);
	}

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	IWRun(const StateModel& model, OpenListT&& open, bool complete, bool mark_negative_propositions) :
		Base(model, std::move(open), ClosedListT()),
		_complete(complete),
		_reached(model.num_subgoals(), nullptr),
		_unreached(),
		_mark_negative_propositions(mark_negative_propositions),
		_visited()
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
		NodePT n = std::make_shared<NodeT>(seed);
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
				NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current );
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

	//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
	//! NOTE - as of yet, this is unused
	bool _complete;

	//! _reached[i] contains the first node that reaches goal atom 'i'.
	std::vector<NodePT> _reached;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	std::unordered_set<unsigned> _unreached;

	bool _mark_negative_propositions;

	//boost::fast_pool_allocator<NodeT> _allocator;
	
	//! Upon retrieval of the set of relevant atoms, this will contain all those nodes that are part
	//! of the path to some subgoal
	std::unordered_set<NodePT> _visited;


	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node(const NodePT& node) {
		const StateT& state = node->state;

		// We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
		// to check if the current node satisfies any of them - and if it does, we mark it appropriately.
		for (auto it = _unreached.begin(); it != _unreached.end(); ) {
			unsigned subgoal_idx = *it;

			if (this->_model.goal(state, subgoal_idx)) {
				_reached[subgoal_idx] = node;
				it = _unreached.erase(it);
			} else {
				++it;
			}
		}
		// As soon as all nodes have been processed, we return true so that we can stop the search
		return _unreached.empty();
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
		for (unsigned subgoal_idx = 0; subgoal_idx < _reached.size(); ++subgoal_idx) {
			NodePT node = _reached[subgoal_idx];
			if (!node) { // No solution for the subgoal was found
				LPT_EDEBUG("simulation-relevant", "Goal atom #" << subgoal_idx << " unreachable");
				continue;
			}
			++reachable;

			// Traverse from the solution node to the root node, adding all atoms on the way
			// if (node->has_parent()) node = node->parent; // (Don't) skip the last node
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				if (_visited.find(node) != _visited.end()) break;
				
				// Mark all the atoms in the state as "yet to be reached"
				atomset.mark(node->state, &(node->parent->state), RelevantAtomSet::STATUS::UNREACHED, _mark_negative_propositions, false);
				_visited.insert(node);
				node = node->parent;
			}
			
			_visited.insert(node); // Insert the root anyway to mark it as a relevant node
		}

		return atomset;
	}
	
	const std::unordered_set<NodePT>& get_relevant_nodes() const { return _visited; }
};

} } // namespaces
