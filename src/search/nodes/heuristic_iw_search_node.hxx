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
class HeuristicIWSearchNode {
public:
	State state;
	GroundAction::IdType action;
	std::shared_ptr<FS0_Node<State> > parent;
	unsigned novelty;
	unsigned g;
	unsigned novelty; // evaluation function
	bool _is_dead_end;
	unsigned num_unsat;

	
public:
	// Kill default constructors
	explicit HeuristicIWSearchNode();
	
	//! Constructor with full copying of the state (expensive)
	HeuristicIWSearchNode(const State& s)
		: state(s), action( GroundAction::invalid_action_id ), parent( nullptr ), novelty(0), g(0), _is_dead_end(false), num_unsat(0) {
	}

	//! Constructor with move of the state (cheaper)
	HeuristicIWSearchNode(State&& _state, GroundAction::IdType _action, std::shared_ptr< HeuristicIWSearchNode<State> > _parent) :
		state(_state), action(_action), parent(_parent), novelty(0), g(_parent->g + 1), _is_dead_end(false), num_unsat(0) {
	}

	virtual ~HeuristicIWSearchNode() {}

	bool has_parent() const { return parent != nullptr; }

	void print( std::ostream& os ) const {
		os << "{@ = " << this << ", s = " << state << ", novelty = " << novelty << ", g = " << g << " unsat = " << num_unsat << ", parent = " << parent << "}";
	}

	bool operator==( const HeuristicIWSearchNode<State>& o ) const { return state == o.state; }

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	void	evaluate_with( Heuristic& heuristic ) {
		novelty = heuristic.evaluate_novelty( state );
		_is_dead_end = novelty > heuristic.novelty_bound();
		num_unsat = heuristic.evaluate_num_unsat_goals( state );
// 		if ( parent != nullptr && num_unsat < parent->num_unsat ) {
			//std::cout << "Reward!" << std::endl;
			//print(std::cout);
			//std::cout << std::endl;
// 		}
		/* NOT USED as this is the heuristic function for gbfs(f)
		h = heuristic.evaluate_reachability( state );
		unsigned ha = 2;
		if ( parent != nullptr && h < parent->h ) ha = 1;
		novelty = 2 * (novelty - 1) + ha;
		*/
	}

	bool dead_end() const { return _is_dead_end; }

	std::size_t hash() const { return state.hash(); }

	// MRJ: With this we implement Greedy Best First modified to be aware of state novelty
	bool operator>( const HeuristicIWSearchNode<State>& other ) const {
		if ( novelty > other.novelty ) return true;
		if ( novelty < other.novelty ) return false;
		return g > other.g;
	}
};


} }  // namespaces
