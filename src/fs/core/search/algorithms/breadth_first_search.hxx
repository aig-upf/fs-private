
#pragma once

#include <fs/core/utils/system.hxx>

#include <lapkt/algorithms/generic_search.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>
#include <lapkt/tools/resources_control.hxx>


namespace lapkt {

//! Partial specialization of the GenericSearch algorithm:
//! A breadth-first search is a generic search with a FIFO open list and 
//! a standard unsorted closed list. Type of node and state model are still generic.
template <typename NodeT,
          typename StateModel,
          typename StatsT
>
class StlBreadthFirstSearch {
public:
    using OpenListT = lapkt::SimpleQueue<NodeT>;
    using ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>;
    using StateT = typename StateModel::StateT;
    using ActionIdT = typename StateModel::ActionType::IdType;
    using PlanT =  std::vector<ActionIdT>;
    using NodePT = std::shared_ptr<NodeT>;


	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	StlBreadthFirstSearch(const StateModel& model, StatsT& stats, bool verbose) :
            _model(model), _open(), _closed(), _generated(0), _stats(stats), _verbose(verbose)
	{}
	
	virtual ~StlBreadthFirstSearch() = default;
	
	// Disallow copy, but allow move
	StlBreadthFirstSearch(const StlBreadthFirstSearch&) = delete;
	StlBreadthFirstSearch(StlBreadthFirstSearch&&) = default;
	StlBreadthFirstSearch& operator=(const StlBreadthFirstSearch&) = delete;
	StlBreadthFirstSearch& operator=(StlBreadthFirstSearch&&) = default;

    float node_generation_rate() {
        return _stats.generated() * 1.0 / (aptk::time_used() - _stats.initial_search_time());
    }

	void on_generation(const NodeT& node) {
        _stats.generation(node.g);

        if (_verbose) {
            auto generated = _stats.generated();
            if (generated % 50000 == 0) {
                LPT_INFO("cout", "Node generation rate after " << generated / 1000 << "K generations (nodes/sec.): " << node_generation_rate()
                                                               << ". Memory consumption: "<< fs0::get_current_memory_in_kb() << "kB. / " << fs0::get_peak_memory_in_kb() << " kB.");
            }
        }
	}

	//! We redefine where the whole search schema following Russell&Norvig.
	//! The only modification is that the check for whether a state is a goal
	//! or not is done right after the creation of the state, instead of upon expansion.
	//! On a problem that has a solution at depth 'd', this avoids the worst-case expansion
	//! of all the $b^d$ nodes of the last (deepest) layer (where b is the branching factor).
	bool search(const StateT& s, PlanT& solution) {
		NodePT n = std::make_shared<NodeT>(s, this->_generated++);

        LPT_INFO("cout", *n);
		
		if (this->check_goal(n, solution)) return true;
		
		this->_open.insert(n);
		
		while (!this->_open.empty()) {
			NodePT current = this->_open.next( );

			for (const auto& a:this->_model.applicable_actions(current->state)) {
				NodePT successor = std::make_shared<NodeT>(
				        this->_model.next(current->state, a), a, current, this->_generated++);

				on_generation(*successor);

                // The node has already been closed, either because it has been expanded, or because it is already
                // in the open list waiting to be expanded
				if (this->_closed.check(successor)) continue;

				if (this->check_goal(successor, solution)) return true;
				
				this->_open.insert(successor);
                this->_closed.put(successor);
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

    //! Convenience method
    bool solve_model(PlanT& solution) { return search( _model.init(), solution ); }

protected:

    virtual bool check_goal(const NodePT& node, PlanT& solution) {
        if (_model.goal(node->state)) { // Solution found, we're done
            if (_verbose) {
                LPT_INFO("search", "Goal found");
            }
            retrieve_solution(node, solution);
            return true;
        }
        return false;
    }

    //! The search model
    const StateModel& _model;

    //! The open list
    OpenListT _open;

    //! The closed list
    ClosedListT _closed;

    //! The number of generated nodes so far
    unsigned long _generated;

    StatsT& _stats;
    bool _verbose;
}; 

}
