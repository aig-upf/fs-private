/*
Lightweight Automated Planning Toolkit (LAPKT)
Copyright (C) 2015

<contributors>
Miquel Ramirez <miquel.ramirez@gmail.com>
</contributors>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <lapkt/algorithms/generic_search.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>


namespace lapkt {

//! Partial specialization of the GenericSearch algorithm:
//! A breadth-first search is a generic search with a FIFO open list and 
//! a standard unsorted closed list. Type of node and state model are still generic.
template <typename NodeT,
          typename StateModel,
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
	
	using NodeOpenEvent = typename BaseClass::NodeOpenEvent;
	using GoalFoundEvent = typename BaseClass::GoalFoundEvent;
	using NodeExpansionEvent = typename BaseClass::NodeExpansionEvent;
	using NodeCreationEvent = typename BaseClass::NodeCreationEvent;

	//! The constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open and closed list objects
	StlBreadthFirstSearch(const StateModel& model, OpenListT&& open) :
		BaseClass(model, std::move(open), ClosedListT())
	{}
	
	//! For convenience, a constructor where the open list is default-constructed
	StlBreadthFirstSearch(const StateModel& model) :
		StlBreadthFirstSearch(model, OpenListT())
	{}
	
	virtual ~StlBreadthFirstSearch() = default;
	
	// Disallow copy, but allow move
	StlBreadthFirstSearch(const StlBreadthFirstSearch&) = delete;
	StlBreadthFirstSearch(StlBreadthFirstSearch&&) = default;
	StlBreadthFirstSearch& operator=(const StlBreadthFirstSearch&) = delete;
	StlBreadthFirstSearch& operator=(StlBreadthFirstSearch&&) = default;
	
	
	//! We redefine where the whole search schema following Russell&Norvig.
	//! The only modification is that the check for whether a state is a goal
	//! or not is done right after the creation of the state, instead of upon expansion.
	//! On a problem that has a solution at depth 'd', this avoids the worst-case expansion
	//! of all the $b^d$ nodes of the last (deepest) layer (where b is the branching factor).
	bool search(const StateT& s, PlanT& solution) override {
		NodePT n = std::make_shared<NodeT>(s, this->_generated++);
		this->notify(NodeCreationEvent(*n));
		
		if (this->check_goal(n, solution)) return true;
		
		this->_open.insert(n);
		
		while (!this->_open.empty()) {
			NodePT current = this->_open.next( );
			this->notify(NodeOpenEvent(*current));
			
			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			this->_closed.put(current);
			
			this->notify(NodeExpansionEvent(*current));
			
			for ( const auto& a : this->_model.applicable_actions( current->state ) ) {
				StateT s_a = this->_model.next( current->state, a );
				NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, this->_generated++);
				
				if (this->_closed.check(successor)) continue; // The node has already been closed
				if (this->_open.contains(successor)) continue; // The node is already in the open list (and surely won't have a worse g-value, this being BrFS)

				this->notify(NodeCreationEvent(*successor));

				if (this->check_goal(successor, solution)) return true;
				
				this->_open.insert( successor );
			}
		}
		return false;
	}
}; 

}
