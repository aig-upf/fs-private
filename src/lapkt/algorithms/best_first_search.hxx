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

#include <memory>

#include <lapkt/algorithms/generic_search.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <lapkt/search/components/stl_unordered_map_closed_list.hxx>

namespace lapkt {

//! Partial specialization of the GenericSearch algorithm:
//! A best-first search is a generic search with an open list sorted by a certain (given) heuristic
//! and a standard unsorted closed list. Type of node and state model are still generic.
template <typename NodeT,
          typename StateModel,
          typename NodePT = std::shared_ptr<NodeT>,
          typename NodeCompareT = node_comparer<NodePT>,
          typename OpenListT = UpdatableOpenList<NodeT, NodePT, NodeCompareT>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeT>
>
class StlBestFirstSearch : public GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>
{
public:
	using BaseClass = GenericSearch<NodeT, OpenListT, ClosedListT, StateModel>;
	
	//! This constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular open list, probably containing some particular heuristic object to sort the ndoes itself.
	StlBestFirstSearch(const StateModel& model, OpenListT&& open) :
		BaseClass(model, std::move(open), ClosedListT())
	{}
	
	//! Simply default-construct an open list
	StlBestFirstSearch(const StateModel& model) :
		BaseClass(model, OpenListT(), ClosedListT())
	{}
	
	virtual ~StlBestFirstSearch() = default;
	
	// Disallow copy, but allow move
	StlBestFirstSearch(const StlBestFirstSearch&) = delete;
	StlBestFirstSearch(StlBestFirstSearch&&) = default;
	StlBestFirstSearch& operator=(const StlBestFirstSearch&) = delete;
	StlBestFirstSearch& operator=(StlBestFirstSearch&&) = default;
}; 

}
