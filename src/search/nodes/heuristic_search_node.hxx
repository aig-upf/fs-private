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
#include <vector>

namespace fs0 { class Atom; }

namespace fs0 { namespace drivers {

template <typename StateT, typename ActionT>
class HeuristicSearchNode {
public:
	using ptr_t = std::shared_ptr<HeuristicSearchNode<StateT, ActionT>>;
	using ActionIdT = typename ActionT::IdType;

	HeuristicSearchNode() = delete;
	~HeuristicSearchNode() = default;
	
	HeuristicSearchNode(const HeuristicSearchNode&) = delete;
	HeuristicSearchNode(HeuristicSearchNode&&) = delete;
	HeuristicSearchNode& operator=(const HeuristicSearchNode&) = delete;
	HeuristicSearchNode& operator=(HeuristicSearchNode&&) = delete;
	
	
	HeuristicSearchNode(const StateT& state_)
		: state(state_), action(ActionT::invalid_action_id), parent(nullptr), g(0), h(std::numeric_limits<long>::max()), _helpful(false)
	{}
	
	HeuristicSearchNode(StateT&& state_, ActionIdT action_, ptr_t parent_) :
		state(std::move(state_)), action(action_), parent(parent_), g(parent_->g + 1), h(std::numeric_limits<long>::max()), _helpful(false)
	{}

	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const HeuristicSearchNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
		std::string h_ = (h == std::numeric_limits<long>::max()) ? "-" : std::to_string(h);
		std::string gh_ = (h == std::numeric_limits<long>::max()) ? "-" : std::to_string(g+h);
		os << "{@ = " << this << ", s = " << state << ", g = " << g << ", h = " << h_ <<  ", g+h = " << gh_ << ", helpful: " << _helpful << ", parent = " << parent << ", action: " << action << "}";
		return os;
	}
	
	//! Forward the comparison and hash function to the search state.
	bool operator==(const HeuristicSearchNode<StateT, ActionT>& o) const { return state == o.state; }
	std::size_t hash() const { return state.hash(); }

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	long evaluate_with(Heuristic& heuristic) {
		h = heuristic.evaluate(state, _relevant);
		LPT_DEBUG("heuristic" , std::endl << "Computed heuristic value of " << h <<  " for state: " << std::endl << state << std::endl << "****************************************");
		
		// If the heuristic is > 0, then we must have a relaxed plan and at least some atoms in the 1st layer of it.
		// TODO But this holds only for certain types of searches, other search engines using this very same node type 
		// won't add anything to the 'relevant' vector
		// assert(h <= 0 || _relevant.size() > 0);
		return h;
	}
	
	void inherit_heuristic_estimate() {
		if (parent) h = parent->h;
	}
	
	//! What to do when an 'other' node is found during the search while 'this' node is already in
	//! the open list
	void update_in_open_list(ptr_t other) {
		if (other->g < this->g) {
			this->g = other->g;
			this->action = other->action;
			this->parent = other->parent;
		}
	}

	//! This effectively implements Greedy Best First search
	bool operator>( const HeuristicSearchNode<StateT, ActionT>& other ) const { return h > other.h; }

	bool dead_end() const { return h == -1; }

	StateT state;
	
	ActionIdT action;
	
	ptr_t parent;
	
	unsigned g;
	
	long h;
	
	bool is_helpful() const { return _helpful; }
	void mark_as_helpful() { _helpful = true; }
	
	const std::vector<Atom>& get_relevant() const { return _relevant; }

protected:
	//! The indexes of the atoms/tuples that were relevant in computing the heuristic of this node.
	std::vector<Atom> _relevant;
	
	bool _helpful;
};

} }  // namespaces
