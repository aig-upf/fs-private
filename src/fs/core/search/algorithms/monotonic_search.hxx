
#pragma once

#include <algorithm>
#include <memory>

#include <lapkt/tools/events.hxx>
#include <fs/core/constraints/gecode/handlers/monotonicity_csp.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/heuristics/unsat_goal_atoms.hxx>

namespace lapkt {

//! A generic search schema
template <typename NodeT,
		typename StateModel,
        typename NodeCompareT = node_comparer<std::shared_ptr<NodeT>>,
    	typename ActionIdT = typename StateModel::ActionType::IdType,
		typename _StateT = typename StateModel::StateT>
class MonotonicSearch : public events::Subject {
public:
	using PlanT =  std::vector<ActionIdT>;
    using NodePT = std::shared_ptr<NodeT>;
	using StateT = _StateT;
	using OpenList = UpdatableOpenList<NodeT, NodePT, NodeCompareT>;
	using ClosedList = aptk::StlUnorderedMapClosedList<NodeT>;
	
	//! Relevant events
	using NodeOpenEvent = events::NodeOpenEvent<NodeT>;
	using GoalFoundEvent = events::GoalFoundEvent<NodeT>;
	using NodeExpansionEvent = events::NodeExpansionEvent<NodeT>;
	using NodeCreationEvent = events::NodeCreationEvent<NodeT>;

	//! The only allowed constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the open list object to be used in the search
	//! (3) the closed list object to be used in the search

	MonotonicSearch(const StateModel& model, fs0::gecode::MonotonicityCSP* monot_manager) :
		_model(model), _goalcounter(model.getTask().getGoalConditions(), model.getTask().get_tuple_index()),
		_open(), _closed(), _generated(0), _monotonicity_csp_manager(monot_manager), _num_pruned(0), _num_deadends(0)
	{}

	virtual ~MonotonicSearch() = default;
	
	// Disallow copy, but allow move
	MonotonicSearch(const MonotonicSearch& other) = delete;
	MonotonicSearch(MonotonicSearch&& other) = default;
	MonotonicSearch& operator=(const MonotonicSearch& rhs) = delete;
	MonotonicSearch& operator=(MonotonicSearch&& rhs) = default;

	virtual bool _search(const StateT& s, PlanT& solution) {
		NodePT n = std::make_shared<NodeT>(s, _generated++);
		this->notify(NodeCreationEvent(*n));


        if (_monotonicity_csp_manager) {
            n->_domains = _monotonicity_csp_manager->create_root(s);
            if (n->_domains.is_null()) {
                LPT_INFO("cout", "Root node detected as inconsistent for monotonicity reasons");
                return false;
            } else {
                LPT_INFO("cout", "Root node is monotonic-consistent");
            }
        }

		_open.insert(n);


		
		while ( !_open.empty() ) {
			NodePT current = _open.next( );
			
			this->notify(NodeOpenEvent(*current));
			
			if (check_goal(current, solution)) return true;

			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			_closed.put(current);
			
			this->notify(NodeExpansionEvent(*current));
			
			for ( const auto& a : _model.applicable_actions( current->state ) ) {
				StateT s_a = _model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, _generated++);
				
				if (_closed.check(successor)) continue; // The node has already been closed
				if (_open.updatable(successor)) continue; // The node is currently on the open list, we update some of its attributes but there's no need to reinsert it.
				
                if (_monotonicity_csp_manager) {
                    assert(!current->_domains.is_null());

                    const auto changeset = _model.get_last_changeset();
                    successor->_domains = _monotonicity_csp_manager->generate_node(
                            current->state,
                            current->_domains,
                            successor->state,
                            changeset
                    );

                    if (successor->_domains.is_null()) {
                        LPT_DEBUG("cout", "\tChildren node pruned because of inconsistent monotonicity CSP. Changes wrt parent were:");
                        LPT_DEBUG("cout", "\t" << fs0::print::changeset(changeset));
                        LPT_DEBUG("cout", "\tFull state was: " << std::endl << "\t" << *successor);
                        _closed.put(successor);
                        ++_num_pruned;
                        continue;
                    } else {
//                        LPT_DEBUG("cout", "Children node is monotonic-consistent:" << std::endl << "\t" << *successor);
                    }
                }

                this->notify(NodeCreationEvent(*successor));

                if (successor->dead_end()) ++_num_deadends;

				successor->unachieved_subgoals = _goalcounter.evaluate(successor->state);

				_open.insert(successor);
			}
		}
		return false;
	}

	//! Backward chaining procedure to recover a plan from a given node
	virtual void retrieve_solution(NodePT node, PlanT& solution) {
		while (node->has_parent()) {
			solution.push_back(node->action);
			node = node->parent;
		}
		std::reverse( solution.begin(), solution.end() );
	}

    //! A simple wrapper
    bool search(const StateT& s, PlanT& solution) {
        auto res = _search(s, solution);
        LPT_INFO("cout", "Nodes pruned by monotonicity constraints: " << _num_pruned);
        LPT_INFO("cout", "Number of deadends: " << _num_deadends);
        return res;
    }

	//! Convenience method
	bool solve_model(PlanT& solution) {
        return search( _model.init(), solution );
    }
	
protected:
	
	virtual bool check_goal(const NodePT& node, PlanT& solution) {
		if ( _model.goal(node->state)) { // Solution found, we're done
			this->notify(GoalFoundEvent(*node));
			retrieve_solution(node, solution);
			return true;
		}
		return false;
	}
	
	//! The search model
	const StateModel& _model;

	fs0::UnsatisfiedGoalAtomsCounter _goalcounter;
	
	//! The open list
	OpenList _open;
	
	//! The closed list
	ClosedList _closed;
	
	//! The number of generated nodes so far
    uint32_t _generated;

    std::unique_ptr<fs0::gecode::MonotonicityCSP> _monotonicity_csp_manager;

    unsigned long _num_pruned;
    unsigned long _num_deadends;

	//* Some methods mainly for debugging purposes
	bool check_open_list_integrity() const {
		OpenList copy(_open);
		while (!copy.empty()) {
			NodePT node = copy.next();
			check_node_correctness(node);
		}
		return true;
	}
	
	bool check_closed_list_integrity() const {
		ClosedList copy(_closed);
		for (auto node:copy) {
			check_node_correctness(node.second);
		}
		return true;
	}
	
	bool check_node_correctness(const NodePT& node) const {
		if (node->has_parent()) {
//			assert(_model.is_applicable(node->parent->state, node->action));
		}
		return true;
	}
};

}
