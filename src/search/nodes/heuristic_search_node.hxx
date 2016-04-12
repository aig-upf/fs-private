/*
FS0 planner
Copyright (c) 2015
Guillem Frances <guillem.frances@upf.edu>


Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>

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

#include <utils/logging.hxx>

namespace fs0 { namespace drivers {

template <typename StateT, typename ActionT>
class HeuristicSearchNode {
public:
	HeuristicSearchNode() = delete;
	~HeuristicSearchNode() {}
	
	HeuristicSearchNode(const HeuristicSearchNode& other) = delete;
	HeuristicSearchNode(const HeuristicSearchNode&& other) = delete;
	HeuristicSearchNode& operator=(const HeuristicSearchNode& rhs) = delete;
	HeuristicSearchNode& operator=(HeuristicSearchNode&& rhs) = delete;
	
	
	HeuristicSearchNode(const StateT& state_)
		: state(state_), action(ActionT::invalid_action_id), parent(nullptr), g(0), h(0)
	{}

	HeuristicSearchNode(StateT&& state_, typename ActionT::IdType action_, std::shared_ptr<HeuristicSearchNode<StateT ,ActionT>> parent_) :
		state(std::move(state_)), action(action_), parent(parent_), g(parent_->g + 1), h(0)
	{}

	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	void print( std::ostream& os ) const { os << "{@ = " << this << ", s = " << state << ", g = " << g << ", h = " << h <<  ", g+h = " << g+h << ", parent = " << parent << ", action: " << action << "}";  }
	
	//! Forward the comparison and hash function to the search state.
	bool operator==(const HeuristicSearchNode<StateT, ActionT>& o) const { return state == o.state; }
	std::size_t hash() const { return state.hash(); }

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	void evaluate_with(Heuristic& heuristic) {
		h = heuristic.evaluate(state);
		FDEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for seed state: " << std::endl << state << std::endl << "****************************************");
	}

	//! This effectively implements Greedy Best First search
	bool operator>( const HeuristicSearchNode<StateT, ActionT>& other ) const { return h > other.h; }

	bool dead_end() const { return h == -1; }

	StateT state;
	
	typename ActionT::IdType action;
	
	std::shared_ptr<HeuristicSearchNode<StateT, ActionT>> parent;
	
	unsigned g;
	
	long h;
};

} }  // namespaces
