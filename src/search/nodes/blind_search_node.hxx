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

#include <aptk2/tools/logging.hxx>
#include <actions/actions.hxx>
#include <state.hxx>

namespace fs0 { namespace drivers {

template <typename StateT, typename ActionT>
class BlindSearchNode {
public:
	StateT state; // TODO - Check no extra copies are being performed, or switch to pointers otherwise.
	
	typename ActionT::IdType action;

	std::shared_ptr<BlindSearchNode<StateT, ActionT>> parent;

public:
	BlindSearchNode() = delete;
	~BlindSearchNode() {}
	
	BlindSearchNode(const BlindSearchNode&) = delete;
	BlindSearchNode(BlindSearchNode&&) = delete;
	BlindSearchNode& operator=(const BlindSearchNode&) = delete;
	BlindSearchNode& operator=(BlindSearchNode&&) = delete;
	
	//! Constructor with full copying of the state (expensive)
	BlindSearchNode( const StateT& s )
		: state( s ), action( ActionT::invalid_action_id ), parent( nullptr )
	{}

	//! Constructor with move of the state (cheaper)
	BlindSearchNode( StateT&& _state, typename ActionT::IdType _action, std::shared_ptr< BlindSearchNode<StateT, ActionT> > _parent ) :
		state(std::move(_state)), action(_action), parent(_parent)
	{}

	bool has_parent() const { return parent != nullptr; }

		//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const BlindSearchNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		os << "{@ = " << this << ", s = " << state << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const BlindSearchNode<StateT, ActionT>& o ) const { return state == o.state; }

	std::size_t hash() const { return state.hash(); }
};

} }  // namespaces
