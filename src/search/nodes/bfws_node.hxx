/*

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

namespace fs0 { namespace drivers {


template <typename StateT, typename ActionT>
class BFWSNode {
public:
	using ptr_t = std::shared_ptr<BFWSNode<StateT, ActionT>>;
	
	StateT state;
	typename ActionT::IdType action;
	
	ptr_t parent;

	//! Accummulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;
	
	//! Number of unsatisfied goal atoms of the state
	unsigned num_unsat;
	
public:
	BFWSNode() = delete;
	~BFWSNode() {}
	
	BFWSNode(const BFWSNode& other) = delete;
	BFWSNode(BFWSNode&& other) = delete;
	BFWSNode& operator=(const BFWSNode& rhs) = delete;
	BFWSNode& operator=(BFWSNode&& rhs) = delete;
	
	//! Constructor with full copying of the state (expensive)
	BFWSNode(const StateT& s) : BFWSNode(StateT(s), ActionT::invalid_action_id, nullptr) {}

	//! Constructor with move of the state (cheaper)
	BFWSNode(StateT&& _state, typename ActionT::IdType action_, ptr_t parent_) :
		state(std::move(_state)), action(action_), parent(parent_), g(parent ? parent->g+1 : 0), novelty(std::numeric_limits<unsigned>::max()), num_unsat(std::numeric_limits<unsigned>::max())
	{}

	bool has_parent() const { return parent != nullptr; }
	
	//! Required for the interface of some algorithms that might prioritise helpful actions.
	bool is_helpful() const { return false; }

	
	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const BFWSNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		os << "{@ = " << this << ", s = " << state << ", novelty = " << novelty << ", g = " << g << ", unsat = " << num_unsat << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const BFWSNode<StateT, ActionT>& o ) const { return state == o.state; }

	template <typename Heuristic>
	void evaluate_with( Heuristic& heuristic ) {
		novelty = heuristic.novelty( state );
		if (novelty > heuristic.novelty_bound()) {
			novelty = std::numeric_limits<unsigned>::max();
		}
		num_unsat = heuristic.evaluate_num_unsat_goals( state );
	}
	
	void inherit_heuristic_estimate() {
		if (parent) {
			novelty = parent->novelty;
			num_unsat = parent->num_unsat;
		}
	}
	
	//! What to do when an 'other' node is found during the search while 'this' node is already in
	//! the open list
	void update_in_open_list(ptr_t other) {
		if (other->g < this->g) {
			this->g = other->g;
			this->action = other->action;
			this->parent = other->parent;
			this->novelty = other->novelty;
			this->num_unsat = other->num_unsat;
		}
	}

// 	bool dead_end() const { return novelty == std::numeric_limits<unsigned>::infinity(); }
	bool dead_end() const { return false; }

	std::size_t hash() const { return state.hash(); }

	//! The ordering of the nodes prioritizes:
	//! (1) nodes with lower novelty, (2) nodes with lower number of unsatisfied goals, (3) nodes with lower accumulated cost
	// (Undelying logic is: return true iff the second element should be popped before the first.)
	bool operator>( const BFWSNode<StateT, ActionT>& other ) const {
		if ( novelty > other.novelty ) return true;
		if ( novelty < other.novelty ) return false;
		if (num_unsat > other.num_unsat) return true;
		if (num_unsat < other.num_unsat) return false;
		return g > other.g;
	}
};


} }  // namespaces
