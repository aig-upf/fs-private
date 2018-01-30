
#pragma once

#include <limits>
#include <vector>

#include <lapkt/tools/logging.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/constraints/gecode/handlers/monotonicity_csp.hxx>
#include <fs/core/utils/printers/helper.hxx>

//namespace fs0 { class Atom; }

namespace fs0 { namespace drivers {

const std::vector<Atom> NIL_VECTOR = {};

template <typename StateT, typename ActionT>
class MonotonicNode {
protected:


public:
	using ptr_t = std::shared_ptr<MonotonicNode<StateT, ActionT>>;
	using ActionIdT = typename ActionT::IdType;

	MonotonicNode() = delete;
	~MonotonicNode() = default;
	
	MonotonicNode(const MonotonicNode&) = delete;
	MonotonicNode(MonotonicNode&&) = delete;
	MonotonicNode& operator=(const MonotonicNode&) = delete;
	MonotonicNode& operator=(MonotonicNode&&) = delete;
	
	
	MonotonicNode(const StateT& state_, uint32_t gen_order)
		: state(state_), action(ActionT::invalid_action_id), parent(nullptr), g(0), h(std::numeric_limits<long>::max()), _gen_order(gen_order)
	{}
	
	MonotonicNode(StateT&& state_, ActionIdT action_, ptr_t parent_, uint32_t gen_order) :
		state(std::move(state_)), action(action_), parent(parent_), g(parent_->g + 1), h(std::numeric_limits<long>::max()), _gen_order(gen_order)
	{}

	bool has_parent() const { return parent != nullptr; }

	//! Print the node into the given stream
	friend std::ostream& operator<<(std::ostream &os, const MonotonicNode<StateT, ActionT>& object) { return object.print(os); }
	std::ostream& print(std::ostream& os) const {
        const auto& problem = Problem::getInstance();
		std::string h_ = (h == std::numeric_limits<long>::max()) ? "-" : std::to_string(h);
		std::string gh_ = (h == std::numeric_limits<long>::max()) ? "-" : std::to_string(g+h);
        std::string porder = (parent != nullptr) ? std::to_string(parent->_gen_order) : "-";
        const auto& act = (action == ActionT::invalid_action_id) ? "-" : fs0::print::to_string(*problem.getGroundActions().at(action));
        os << "{#" << _gen_order << ", @"<< this << ", s = " << state << ", #g = " << unachieved_subgoals << ", g = " << g << ", h = " << h_ <<  ", g+h = " << gh_ << ", parent = " << porder << ", action: " << act << "}";
		return os;
	}
	
	//! Forward the comparison and hash function to the search state.
	bool operator==(const MonotonicNode<StateT, ActionT>& o) const { return state == o.state; }
	std::size_t hash() const { return state.hash(); }

	// MRJ: This is part of the required interface of the Heuristic
	template <typename Heuristic>
	long evaluate_with(Heuristic& heuristic) {
		h = heuristic.evaluate(state);
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
	bool operator>( const MonotonicNode<StateT, ActionT>& other ) const {
		return h > other.h;

//        if (h > other.h) return true;
//        if (h < other.h) return false;
//
//        if (unachieved_subgoals > other.unachieved_subgoals) return true;
//        if (unachieved_subgoals < other.unachieved_subgoals) return false;

//		if (g > other.g) return true;
//		if (g < other.g) return false;
//		return (g > other.g);
	}

	bool dead_end() const {
        return h == -1;
    }

	StateT state;
	
	ActionIdT action;
	
	ptr_t parent;
	
	unsigned g;

	unsigned unachieved_subgoals;
	
	long h;
	
	bool is_helpful() const { return false; }
	void mark_as_helpful() { }
	
	const std::vector<Atom>& get_relevant() const { return NIL_VECTOR; }

    DomainTracker _domains;

	uint32_t _gen_order;
};

} }  // namespaces
