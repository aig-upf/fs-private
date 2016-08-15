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

namespace fs0 { namespace drivers {


template <typename State>
class GBFSNoveltyNode {
public:
	State state;
	GroundAction::IdType action;
	
	std::shared_ptr<GBFSNoveltyNode<State> > parent;

	//! Accummulated cost
	unsigned g;

	//! Novelty of the state
	unsigned novelty;
	
	//! Number of unsatisfied goal atoms of the state
	unsigned num_unsat;
	
public:
	GBFSNoveltyNode() = delete;
	~GBFSNoveltyNode() {}
	
	GBFSNoveltyNode(const GBFSNoveltyNode& other) = delete;
	GBFSNoveltyNode(GBFSNoveltyNode&& other) = delete;
	GBFSNoveltyNode& operator=(const GBFSNoveltyNode& rhs) = delete;
	GBFSNoveltyNode& operator=(GBFSNoveltyNode&& rhs) = delete;
	
	//! Constructor with full copying of the state (expensive)
	GBFSNoveltyNode(const State& s)
		: state(s), action(GroundAction::invalid_action_id), parent(nullptr), g(0), novelty(0), num_unsat(0)
	{}

	//! Constructor with move of the state (cheaper)
	GBFSNoveltyNode(State&& _state, GroundAction::IdType _action, std::shared_ptr< GBFSNoveltyNode<State> > _parent) :
		state(std::move(_state)), action(_action), parent(_parent), g(_parent->g + 1), novelty(0), num_unsat(0)
	{}

	bool has_parent() const { return parent != nullptr; }
	
	//! Required for the interface of some algorithms that might prioritise helpful actions.
	bool is_helpful() const { return false; }

	
	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const GBFSNoveltyNode<State>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const { 
		os << "{@ = " << this << ", s = " << state << ", novelty = " << novelty << ", g = " << g << " unsat = " << num_unsat << ", parent = " << parent << "}";
		return os;
	}

	bool operator==( const GBFSNoveltyNode<State>& o ) const { return state == o.state; }

	template <typename Heuristic>
	void evaluate_with( Heuristic& heuristic ) {
		novelty = heuristic.novelty( state );
		if (novelty > heuristic.novelty_bound()) novelty = std::numeric_limits<unsigned>::infinity();
		num_unsat = heuristic.evaluate_num_unsat_goals( state );
	}
	
	void inherit_heuristic_estimate() {
		if (parent) {
			novelty = parent->novelty;
			num_unsat = parent->num_unsat;
		}
	}

	bool dead_end() const { return novelty == std::numeric_limits<unsigned>::infinity(); }

	std::size_t hash() const { return state.hash(); }

	//! The ordering of the nodes prioritizes:
	//! (1) nodes with lower novelty, (2) nodes with lower number of unsatisfied goals, (3) nodes with lower accumulated cost
	bool operator>( const GBFSNoveltyNode<State>& other ) const {
		if ( novelty > other.novelty ) return true;
		if ( novelty < other.novelty ) return false;
		if (num_unsat > other.num_unsat) return true;
		if (num_unsat < other.num_unsat) return true;
		return g > other.g;
	}
};


} }  // namespaces
