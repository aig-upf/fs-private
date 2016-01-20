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
class BlindSearchNode {
public:
	State state; // TODO - Check no extra copies are being performed, or switch to pointers otherwise.
	fs0::GroundAction::IdType action;
	std::shared_ptr<BlindSearchNode<State>> parent;

public:
	BlindSearchNode() = delete;
	~BlindSearchNode() {}
	
	BlindSearchNode(const BlindSearchNode& other) = delete;
	BlindSearchNode(const BlindSearchNode&& other) = delete;
	BlindSearchNode& operator=(const BlindSearchNode& rhs) = delete;
	BlindSearchNode& operator=(BlindSearchNode&& rhs) = delete;
	
	//! Constructor with full copying of the state (expensive)
	BlindSearchNode( const State& s )
		: state( s ), action( fs0::GroundAction::invalid_action_id ), parent( nullptr )
	{}

	//! Constructor with move of the state (cheaper)
	BlindSearchNode( State&& _state, fs0::GroundAction::IdType _action, std::shared_ptr< BlindSearchNode<State> > _parent ) :
		state(std::move(_state)) {
		action = _action;
		parent = _parent;
	}

	bool has_parent() const { return parent != nullptr; }

	void print( std::ostream& os ) const {
		os << "{@ = " << this << ", s = " << state << ", parent = " << parent << "}";
	}

	bool operator==( const BlindSearchNode<State>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};

} }  // namespaces
