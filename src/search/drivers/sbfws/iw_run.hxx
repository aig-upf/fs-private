
#pragma once

#include <iomanip>

#include <aptk2/search/components/stl_unsorted_fifo_open_list.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>
#include <search/algorithms/aptk/generic_search.hxx>
#include <search/drivers/bfws/iw_novelty_evaluator.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <utils/printers/vector.hxx>
#include <utils/printers/relevant_atomset.hxx>
#include <state.hxx>
#include <boost/pool/pool_alloc.hpp>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace bfws {

//! A RelevantAtomSet contains information about which of the atoms of a problem are relevant for a certain
//! goal, and, among those, which have already been reached and which others have not.
class RelevantAtomSet {
public:
	enum class STATUS : unsigned char {IRRELEVANT, UNREACHED, REACHED};

	RelevantAtomSet(const AtomIndex* atomidx) :
		_atomidx(atomidx), _num_reached(0), _num_unreached(0), _status(atomidx ? atomidx->size() : 0, STATUS::IRRELEVANT)
	{}

	~RelevantAtomSet() = default;
	RelevantAtomSet(const RelevantAtomSet&) = default;
	RelevantAtomSet(RelevantAtomSet&&) = default;
	RelevantAtomSet& operator=(const RelevantAtomSet&) = default;
	RelevantAtomSet& operator=(RelevantAtomSet&&) = default;

	inline void mark(VariableIdx variable, ObjectIdx value, STATUS status, bool only_if_relevant) {
		assert(_atomidx);
		mark(_atomidx->to_index(variable, value), status, only_if_relevant);
	}

	//! A helper
	void mark(const State& state, const State* parent, STATUS status, bool mark_negative_propositions, bool only_if_relevant) {
		const ProblemInfo& info = ProblemInfo::getInstance();
		for (VariableIdx var = 0; var < state.numAtoms(); ++var) {
			ObjectIdx val = state.getValue(var);
			if (!mark_negative_propositions && info.isPredicativeVariable(var) && val==0) continue; // We don't want to mark negative propositions
			if (parent && val == parent->getValue(var)) continue; // If a parent was provided, we check that the value is new wrt the parent
			mark(var, state.getValue(var), status, only_if_relevant);
		}
	}

	void mark(AtomIdx idx, STATUS status, bool only_if_relevant) {
		assert(status==STATUS::REACHED || status==STATUS::UNREACHED);
		auto& st = _status[idx];
		if (only_if_relevant && st == STATUS::IRRELEVANT) return;

		if (st != status) {
			st = status;
			if (status==STATUS::REACHED) ++_num_reached;
			else if (status==STATUS::UNREACHED) ++_num_unreached;
		}
	}

	// inline void reach(VariableIdx variable, ObjectIdx value) { mark(variable, value, STATUS::REACHED); }
	// inline void unreach(VariableIdx variable, ObjectIdx value) { mark(variable, value, STATUS::UNREACHED); }

	unsigned num_reached() const { return _num_reached; }
	unsigned num_unreached() const { return _num_unreached; }

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
	FeatureValuation feature_valuation;

	//! The action that led to this node
	typename ActionT::IdType action;

	//! The parent node
	PT parent;


	IWRunNode() = default;
	~IWRunNode() = default;
	IWRunNode(const IWRunNode&) = default;
	IWRunNode(IWRunNode&&) = delete;
	IWRunNode& operator=(const IWRunNode&) = delete;
	IWRunNode& operator=(IWRunNode&&) = delete;

	//! Constructor with full copying of the state (expensive)
	IWRunNode(const StateT& s)
		: state( s ), action( ActionT::invalid_action_id ), parent( nullptr )
	{}

	//! Constructor with move of the state (cheaper)
	IWRunNode(StateT&& _state, typename ActionT::IdType _action, PT& _parent) :
		state(std::move(_state)),
		action(_action),
		parent(_parent)
	{}

	//! The novelty type (for the IWRun node, will always be 0)
	unsigned type() const { return 0; }

	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "{@ = " << this << ", s = " << state << ", features= " << fs0::print::container(feature_valuation) << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const IWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};


//! Helper
std::vector<Atom> obtain_goal_atoms(const fs::Formula* goal);

//! This is the acceptor for an open list with width-based node pruning
//! - a node is pruned iff its novelty is higher than a given threshold.
class IWRunAcceptor {
protected:
	//! A single novelty evaluator will be in charge of evaluating all nodes
	IWNoveltyEvaluator _novelty_evaluator;

public:
	IWRunAcceptor(const IWNoveltyEvaluator& novelty_evaluator) :
		_novelty_evaluator(novelty_evaluator) // Copy the evaluator
	{}

	~IWRunAcceptor() = default;

	//! Returns false iff we want to prune this node during the search
	template <typename NodeT>
	bool accept(NodeT& node) {
		assert(node.feature_valuation.empty()); // We should not be computing the feature valuation twice!
		node.feature_valuation = _novelty_evaluator.compute_valuation(node.state);
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
          typename OpenListT = aptk::StlUnsortedFIFO<NodeT, IWRunAcceptor>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class IWRun : public lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>
{
public:
	using ActionT = typename StateModel::ActionType;
	using Base = lapkt::GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	using StateT = typename StateModel::StateType;
	using PlanT = typename Base::PlanT;
	using NodePT = typename Base::NodePtr;

	using NodeOpenEvent = typename Base::NodeOpenEvent;
	using NodeExpansionEvent = typename Base::NodeExpansionEvent;
	using NodeCreationEvent = typename Base::NodeCreationEvent;


	//! Factory method
	static IWRun* build(const StateModel& model, const IWNoveltyEvaluator& novelty_evaluator, bool mark_negative_propositions) {
		const Problem& problem = model.getTask();

		auto atoms = obtain_goal_atoms(problem.getGoalConditions());
		auto acceptor = std::make_shared<IWRunAcceptor>(novelty_evaluator);

		return new IWRun(model, OpenListT(acceptor), atoms, false, mark_negative_propositions);
	}

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	IWRun(const StateModel& model, OpenListT&& open, const std::vector<Atom>& goal, bool complete, bool mark_negative_propositions) :
		Base(model, std::move(open), ClosedListT()),
		_complete(complete),
		_goal_atoms(goal),
		_reached(_goal_atoms.size(), nullptr),
		_unreached(),
		_mark_negative_propositions(mark_negative_propositions)
	{

		for (unsigned i = 0; i < _goal_atoms.size(); ++i) _unreached.insert(i); // Initially all goal atoms assumed to be unreached
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
		//NodePT n = std::make_shared<NodeT>(seed);
		NodePT n = std::allocate_shared<NodeT>(_allocator, seed);
		this->notify(NodeCreationEvent(*n));
		this->_open.insert(n);

		while (!this->_open.is_empty()) {
			NodePT current = this->_open.get_next( );
			this->notify(NodeOpenEvent(*current));

			bool all_goals_reached = process_node(current);
			if (all_goals_reached) break;

			// close the node before the actual expansion so that children which are identical to 'current' get properly discarded.
			this->_closed.put(current);

			this->notify(NodeExpansionEvent(*current));

			for (const auto& a : this->_model.applicable_actions(current->state)) {
				StateT s_a = this->_model.next( current->state, a );
				//NodePT successor = std::make_shared<NodeT>( std::move(s_a), a, current );
				NodePT successor = std::allocate_shared<NodeT>( _allocator, std::move(s_a), a, current );

				if (this->_closed.check(successor)) continue; // The node has already been closed

				this->notify(NodeCreationEvent(*successor));
				if (!this->_open.insert( successor )) {
					LPT_DEBUG("search", std::setw(7) << "PRUNED: " << *successor);
				}
			}
		}
	}

protected:

	//! Whether to perform a complete run or a partial one, i.e. up until (independent) satisfaction of all goal atoms.
	//! NOTE - as of yet, this is unused
	bool _complete;

	const std::vector<Atom> _goal_atoms;

	//! _reached[i] contains the first node that reaches goal atom 'i'.
	std::vector<NodePT> _reached;

	//! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
	std::unordered_set<unsigned> _unreached;

	bool _mark_negative_propositions;

	boost::fast_pool_allocator<NodeT>	_allocator;


	//! Returns true iff all goal atoms have been reached in the IW search
	bool process_node(const NodePT& node) {
		const StateT& state = node->state;

		// We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
		// to check if the current node satisfies any of them - and if it does, we mark it appropriately.
		for (auto it = _unreached.begin(); it != _unreached.end(); ) {
			unsigned atom_idx = *it;
			const Atom& atom = _goal_atoms[atom_idx];

			if (state.contains(atom)) { // The state satisfies goal atom with index 'i'
				_reached[atom_idx] = node;
				it = _unreached.erase(it);
				// LPT_INFO("cout", "IWRUN: Goal atom '" << *atom << "' reached for the first time");
			} else {
				++it;
			}
		}
		// As soon as all nodes have been processed, we return true so that we can stop the search
		return _unreached.empty();
	}

public:
	//! Retrieve the set of atoms which are relevant to reach at least one of the subgoals
	RelevantAtomSet retrieve_relevant_atoms(const StateT& seed, unsigned& reachable) const {
		const AtomIndex& atomidx = this->_model.getTask().get_tuple_index();
		RelevantAtomSet atomset(&atomidx);

		LPT_EDEBUG("simulation-relevant", "Computing set of relevant atoms from state: " << std::endl << seed << std::endl);

		// atomset.mark(seed, RelevantAtomSet::STATUS::UNREACHED); // This is not necessary, since all these atoms will be made true by the "root" state of the simulation

		std::unordered_set<NodePT> processed;

		for (unsigned subgoal_idx = 0; subgoal_idx < _reached.size(); ++subgoal_idx) {
			NodePT node = _reached[subgoal_idx];
			if (!node) { // No solution for the subgoal was found
				LPT_EDEBUG("simulation-relevant", "Goal atom '" << _goal_atoms[subgoal_idx] << "' unreachable");
				continue;
			}
			++reachable;

			// Traverse from the solution node to the root node, adding all atoms on the way
			// if (node->has_parent()) node = node->parent; // (Don't) skip the last node
			while (node->has_parent()) {
				// If the node has already been processed, no need to do it again, nor to process the parents,
				// which will necessarily also have been processed.
				if (processed.find(node) != processed.end()) break;
				atomset.mark(node->state, &(node->parent->state), RelevantAtomSet::STATUS::UNREACHED, _mark_negative_propositions, false);
				processed.insert(node);
				node = node->parent;
			}
		}
		LPT_EDEBUG("simulation-relevant", atomset.num_unreached() << " relevant atoms (" << reachable << "/" << _reached.size() << " reachable subgoals): " << print::relevant_atomset(atomset) << std::endl << std::endl);

		return atomset;
	}
};

} } // namespaces
