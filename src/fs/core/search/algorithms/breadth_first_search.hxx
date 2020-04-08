
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
          typename StatsT,
        typename OpenListT = lapkt::SearchableQueue<NodeT>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class StlBreadthFirstSearch : public GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>
{
public:
	using BaseClass = GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	using StateT = typename BaseClass::StateT;
	using PlanT = typename BaseClass::PlanT;
	using NodePT = typename BaseClass::NodePT;
	

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	StlBreadthFirstSearch(const StateModel& model, OpenListT&& open, StatsT& stats, bool verbose) :
		BaseClass(model, std::move(open), ClosedListT()), _stats(stats), _verbose(verbose)
	{}
	
	//! For convenience, a constructor where the open list is default-constructed
	StlBreadthFirstSearch(const StateModel& model, StatsT& stats, bool verbose) :
		StlBreadthFirstSearch(model, OpenListT(), stats, verbose)
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
	bool search(const StateT& s, PlanT& solution) override {
		NodePT n = std::make_shared<NodeT>(s, this->_generated++);

        LPT_INFO("cout", *n);
		
		if (this->check_goal(n, solution)) return true;
		
		this->_open.insert(n);
		
		while (!this->_open.empty()) {
			NodePT current = this->_open.next( );

			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			this->_closed.put(current);
			
			for ( const auto& a : this->_model.applicable_actions( current->state ) ) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, this->_generated++);

				on_generation(*successor);

				if (this->_closed.check(successor)) continue; // The node has already been closed
				if (this->_open.contains(successor)) continue; // The node is already in the open list (and surely won't have a worse g-value, this being BrFS)

				if (this->check_goal(successor, solution)) return true;
				
				this->_open.insert( successor );
			}
		}
		return false;
	}

protected:
    StatsT& _stats;
    bool _verbose;
}; 

}
