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

namespace aptk { namespace search {


template <typename State>
class FS0_Node {
public:
	typedef State State_Type;

	// Kill default constructors
	explicit FS0_Node();
	//! Constructor for initial nodes, copies state
	FS0_Node( const State& s )
		: state( s ), action( fs0::GroundAction::invalid_action_id ), parent( nullptr ), g(0), is_dead_end(false) {
	}

	//! Constructor for successor states, doesn't copy the state
	FS0_Node( State&& _state, fs0::GroundAction::IdType _action, std::shared_ptr< FS0_Node<State> > _parent ) :
		state(_state) {
		action = _action;
		parent = _parent;
		g = _parent->g + 1;
		is_dead_end = false;
	}

	virtual ~FS0_Node() {}

	bool has_parent() const { return parent != nullptr; }

	void print( std::ostream& os ) const { os << "{@ = " << this << ", s = " << state << ", h = " << h << ", parent = " << parent << "}";  }

	//! Forward the comparison to the planning state.
	bool operator==(const FS0_Node<State>& o) const { return state == o.state; }

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	void evaluate_with(Heuristic& heuristic) {
		float _h = heuristic.evaluate(state);
		FDEBUG("heuristic" , std::endl << "Computed heuristic value of " << _h <<  " for seed state: " << std::endl << state << std::endl << "****************************************");
		if ( _h == std::numeric_limits<float>::infinity() ) {
			is_dead_end = true;
		}
		h = _h;
	}

	size_t hash() const { return state.hash(); }

	// MRJ: With this we implement Greedy Best First search
	bool operator>( const FS0_Node<State>& other ) const { return h > other.h; }

	bool dead_end() const { return is_dead_end; }

public:
	State state; // TODO - Check no extra copies are being performed, or switch to pointers otherwise.
	fs0::GroundAction::IdType action;
	std::shared_ptr<FS0_Node<State>> parent;
	unsigned h;
	unsigned g;
	bool is_dead_end;
};

} }  // namespaces
