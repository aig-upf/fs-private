
#pragma once

#include <search/nodes/heuristic_search_node.hxx>
#include <models/ground_state_model.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <utils/printers/vector.hxx>

#include <lapkt/algorithms/breadth_first_search.hxx>
#include <search/events.hxx>
#include <search/stats.hxx>
#include <search/drivers/setups.hxx>


namespace fs0 { namespace drivers {


//! A specialization of a search node tailored for our EHC search; most importantly,
//! we store in the node the set of atoms that are relevant to the first layer of the
//! relaxed plan.
template <typename StateT, typename ActionT>
class EHCSearchNode {
public:
	~EHCSearchNode() = default;

	EHCSearchNode(const EHCSearchNode&) = delete;
	EHCSearchNode(EHCSearchNode&&) = delete;
	EHCSearchNode& operator=(const EHCSearchNode&) = delete;
	EHCSearchNode& operator=(EHCSearchNode&&) = delete;


	EHCSearchNode(StateT&& state_, typename ActionT::IdType action_, std::shared_ptr<EHCSearchNode<StateT, ActionT>> parent_, unsigned long gen_order = 0) :
		state(std::move(state_)), action(action_), parent(parent_), h(0)
	{}

	EHCSearchNode(const StateT& state, unsigned long gen_order = 0) :
		EHCSearchNode(StateT(state), ActionT::invalid_action_id, nullptr)
	{}

	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const EHCSearchNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		os << "{@ = " << this << ", s = " << state << ", h = " << h << ", parent = " << parent << ", action: " << action << "}";
		return os;
	}

	//! Forward the comparison and hash function to the search state.
	bool operator==(const EHCSearchNode<StateT, ActionT>& o) const { return state == o.state; }
	std::size_t hash() const { return state.hash(); }

	template <typename Heuristic>
	long evaluate_with(Heuristic& heuristic) {
		h = heuristic.evaluate(state, _relevant);
		LPT_DEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for state: " << std::endl << state << std::endl << "****************************************");

		// If the heuristic is > 0, then we must have a relaxed plan and at least some atoms in the 1st layer of it.
		assert(h <= 0 || _relevant.size() > 0);
		return h;
	}

	const typename ActionT::IdType& get_action() const { return action; }

	const StateT& get_state() const { return state; }

	std::shared_ptr<EHCSearchNode<StateT, ActionT>> get_parent() { return parent; }

	long get_h() const { return h; }

	const std::vector<Atom>& get_relevant() const { return _relevant; }

	StateT state;

	typename ActionT::IdType action;

	std::shared_ptr<EHCSearchNode<StateT, ActionT>> parent;

	long h;

	bool is_helpful() const { return _helpful; }
	void mark_as_helpful() { _helpful = true; }

protected:
	//! The indexes of the atoms/tuples that were relevant in computing the heuristic of this node.
	std::vector<Atom> _relevant;

	bool _helpful;
};

//! This is a specialized version of breadth-first search that incorporates two modifications:
//!    (1) It aborts (and returns the node) as soon as a node is found with heuristic smaller than a given bound, and
//!    (2) When expanding a node, it prunes those actions that do not satisfy a certain helpful-action criteria, namely
//!        only those actions that add at least one of the supports of the actions in the first block of the relaxed plan.
template <typename StateModel,
          typename HeuristicT,
          typename NodeType = EHCSearchNode<State, GroundAction>
>
class EHCBreadthFirstSearch : public lapkt::StlBreadthFirstSearch<NodeType, StateModel>
{
public:
	using Base = lapkt::StlBreadthFirstSearch<NodeType, StateModel>;
	using NodePtr = std::shared_ptr<NodeType>;
	using PlanT = typename Base::PlanT;

	//! Relevant events
	using NodeOpenEvent = lapkt::events::NodeOpenEvent<NodeType>;
	using GoalFoundEvent = lapkt::events::GoalFoundEvent<NodeType>;
	using NodeExpansionEvent = lapkt::events::NodeExpansionEvent<NodeType>;
	using NodeCreationEvent = lapkt::events::NodeCreationEvent<NodeType>;


	//! For convenience, a constructor where the open list is default-constructed
	EHCBreadthFirstSearch(const StateModel& model, HeuristicT& heuristic, bool prune_unhelpful) :
		Base(model), _heuristic(heuristic), _prune_unhelpful(prune_unhelpful)
	{}

	~EHCBreadthFirstSearch() = default;
	EHCBreadthFirstSearch(const EHCBreadthFirstSearch&) = default;
	EHCBreadthFirstSearch(EHCBreadthFirstSearch&&) = default;
	EHCBreadthFirstSearch& operator=(const EHCBreadthFirstSearch&) = default;
	EHCBreadthFirstSearch& operator=(EHCBreadthFirstSearch&&) = default;


	bool search(const State& state, PlanT& solution) override {
		auto node = make_node(state);
		auto end = bounded_search(node, node->get_h());
		if (end) {
			this->retrieve_solution(end, solution);
		}
		return !!end;
	}

	static NodePtr make_node(const State& state, HeuristicT& heuristic) {
		auto node = std::make_shared<NodeType>(state);
		node->evaluate_with(heuristic);
		return node;
	}

	NodePtr make_node(const State& state) const { return make_node(state, _heuristic); }

	//! Returns the first node with heuristic h < h_bound
	NodePtr bounded_search(NodePtr root, long h_bound) {
		this->_open.insert(root);

		NodePtr goal = nullptr;
		unsigned pruned = 0;
		while (!goal && !this->_open.empty()) {

			NodePtr current = this->_open.next();
			this->notify(NodeOpenEvent(*current));
			this->_closed.put(current);

			this->notify(NodeExpansionEvent(*current));

			// Expand children nodes
			for (const auto& a : Base::_model.applicable_actions(current->get_state(), true)) {
				State s_a = Base::_model.next( current->get_state(), a );
				NodePtr successor = std::make_shared<NodeType>(std::move(s_a), a, current);

				if (this->_closed.check(successor)) continue;

				this->notify(NodeCreationEvent(*successor));

				// If using HA; we check that the current expansion is helpful, if not, discard it.
				if (_prune_unhelpful && !successor->is_helpful()) {
					++pruned;
					continue;
				}

				if (successor->evaluate_with(_heuristic) < h_bound) {
					goal = successor;
					break;
				}

				this->_open.insert( successor );
			}
		}

		LPT_EDEBUG("ehc", "ΕΗC's BrFS search from node " << root->hash() << " pruned " << pruned << " nodes based on helpful-action analysis");
		return goal;
	}

protected:
	//!
	HeuristicT& _heuristic;

	bool _prune_unhelpful;
};


//! This is Enhanced Hill-Climbing using a generic heuristic evaluator and a specialized
//! EHCBreadthFirstSearch breadth-first search algorithm that aborts when a state with
//! lower heuristic is found and performs helpful-action-based pruning
template <typename HeuristicT>
class EHCSearch {
public:
	//! EHC uses a breadth-first search as a base.
	using BreadthFirstAlgorithm = EHCBreadthFirstSearch<GroundStateModel, HeuristicT>;
	using NodeT = EHCSearchNode<State, GroundAction>;

	~EHCSearch() = default;
	EHCSearch(const EHCSearch&) = default;
	EHCSearch(EHCSearch&&) = default;
	EHCSearch& operator=(const EHCSearch&) = default;
	EHCSearch& operator=(EHCSearch&&) = default;

	EHCSearch(const GroundStateModel& model, HeuristicT&& heuristic, bool prune_unhelpful, SearchStats& stats) :
		_model(model), _heuristic(std::move(heuristic)), _prune_unhelpful(prune_unhelpful), _stats(stats)
	{
		EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
		EventUtils::setup_HA_observer<NodeT>(_handlers);
	}

	bool search(const State& state, std::vector<unsigned>& solution) {
		assert(solution.size()==0);

		auto node = BreadthFirstAlgorithm::make_node(state, _heuristic);
		LPT_INFO("cout", "Starting EHC search on node " << *node);

		while(node->h > 0) {

			// Perform breadth-first search until a state with smaller heuristic value is found
			BreadthFirstAlgorithm bfs(_model, _heuristic, _prune_unhelpful);
			lapkt::events::subscribe(bfs, _handlers);

			if (! (node = bfs.bounded_search(node, node->h))) { // EHC fails
				LPT_INFO("cout", "EHC's breadth-first search unable to find a state with lower h(s)");
				return false;
			}

			LPT_INFO("cout", "EHC switch - new search node " << *node);

			if (node->h == 0) bfs.retrieve_solution(node, solution);
		}

		return node->h == 0;
	}

	unsigned long expanded = 0;
	unsigned long generated = 0;

protected:
	//!
	const GroundStateModel& _model;

	//!
	HeuristicT _heuristic;

	//! Whether to prune those actions that are not considered helpful or not
	bool _prune_unhelpful;

	SearchStats& _stats;

	std::vector<std::unique_ptr<lapkt::events::EventHandler>> _handlers;
};




} } // namespaces
