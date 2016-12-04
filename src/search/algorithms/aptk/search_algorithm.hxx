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

Additional note:

Concepts borrowed from Ethan Burn's heuristic search framework.
*/

#pragma once

#include <vector>
#include <iostream>

#include <aptk2/tools/logging.hxx>

namespace lapkt {

class SearchStatistics {};

// A pure interface for specifying a search algorithm that is able to 
template <typename StateT, typename ActionT>
class SearchAlgorithm {
public:
	using PlanT   = std::vector<ActionT>;

	virtual ~SearchAlgorithm() = default;

	//! This method encapsulates the actual search for a plan achieving the goal from the
	//! provided state s. Argument solution will be empty if there is no valid plan.
	virtual bool search(const StateT& s, PlanT& solution) = 0;
	
	//!
	virtual void stats(SearchStatistics&) {};
};

}
