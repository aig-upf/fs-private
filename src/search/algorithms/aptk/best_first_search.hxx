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

#include <search/algorithms/aptk/generic_search.hxx>
#include <search/algorithms/aptk/sorted_open_list.hxx>
#include <aptk2/search/components/stl_unordered_map_closed_list.hxx>

namespace lapkt {

//! Partial specialization of the GenericSearch algorithm:
//! A best-first search is a generic search with an open list sorted by a certain (given) heuristic
//! and a standard unsorted closed list. Type of node and state model are still generic.
template <typename NodeType,
          typename Heuristic,
          typename StateModel,
          typename OpenListT = StlSortedOpenList<NodeType, Heuristic>,
          typename ClosedListT = aptk::StlUnorderedMapClosedList<NodeType>
>
class StlBestFirstSearch : public GenericSearch<NodeType, OpenListT, ClosedListT, StateModel>
{
public:
	using BaseClass = GenericSearch<NodeType, OpenListT, ClosedListT, StateModel>;
	
	//! The only allowed constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the particular heuristic object to be used to evaluate states
	StlBestFirstSearch(const StateModel& model, Heuristic& heuristic) :
		BaseClass(model, OpenListT(heuristic), ClosedListT())
	{}
	
	virtual ~StlBestFirstSearch() = default;
	
	// Disallow copy, but allow move
	StlBestFirstSearch(const StlBestFirstSearch&) = delete;
	StlBestFirstSearch(StlBestFirstSearch&&) = default;
	StlBestFirstSearch& operator=(const StlBestFirstSearch&) = delete;
	StlBestFirstSearch& operator=(StlBestFirstSearch&&) = default;
}; 

}
