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
#include <actions/ground_action.hxx>

namespace fs0 { namespace engines {

template <typename State>
class HeuristicSearchNode {
public:
	typedef State State_Type;

	//! Kill default constructors
	explicit HeuristicSearchNode();
	
	//! Constructor with full copying of the state (expensive)
	HeuristicSearchNode(const State& state_)
		: state(state_), action(fs0::GroundAction::invalid_action_id), parent(nullptr), g(0), h(0)
	{}

	//! Constructor with move of the state (cheaper)
	HeuristicSearchNode(State&& state_, fs0::GroundAction::IdType action_, std::shared_ptr<HeuristicSearchNode<State>> parent_) :
		state(state_), action(action_), parent(parent_), g(parent_->g + 1), h(0)
	{}

	virtual ~HeuristicSearchNode() {}

	bool has_parent() const { return parent != nullptr; }

	void print( std::ostream& os ) const { os << "{@ = " << this << ", s = " << state << ", h = " << h << ", parent = " << parent << "}";  }

	//! Forward the comparison to the planning state.
	bool operator==(const HeuristicSearchNode<State>& o) const { return state == o.state; }

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	void evaluate_with(Heuristic& heuristic) {
		h = heuristic.evaluate(state);
		FDEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for seed state: " << std::endl << state << std::endl << "****************************************");
	}

	std::size_t hash() const { return state.hash(); }

	// MRJ: With this we implement Greedy Best First search
	bool operator>( const HeuristicSearchNode<State>& other ) const { return h > other.h; }

	bool dead_end() const { return h == -1; }

	State state; // TODO - Check no extra copies are being performed, or switch to pointers otherwise.
	
	fs0::GroundAction::IdType action;
	
	std::shared_ptr<HeuristicSearchNode<State>> parent;
	
	unsigned g;
	
	long h;
};

} }  // namespaces
