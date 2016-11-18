/*
Lightweight Automated Planning Toolkit (LAPKT)
Copyright (C) 2015

<contributors>
Miquel Ramirez <miquel.ramirez@gmail.com>
Guillem Francès <guillem.frances@gmail.com>
</contributors>

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

Additional note:

Concepts borrowed from Ethan Burn's heuristic search framework.
*/

#pragma once

#include <algorithm>
#include <memory>

#include <search/algorithms/aptk/search_algorithm.hxx>
#include <search/algorithms/aptk/events.hxx>
#include <problem_info.hxx>
#include <utils/printers/actions.hxx>
#include <problem.hxx>
#include <actions/action_id.hxx>
#include <aptk2/tools/logging.hxx>

namespace lapkt {

template <typename NodePT>
struct NullPruner {
	static bool inline prune(const NodePT& node) { return false; } // By default don't prune any node
};

//! A generic search schema
template <typename NodeType,
          typename OpenList,
          typename ClosedList,
          typename StateModel,
          typename StateT = typename StateModel::StateType,
          typename ActionT = typename StateModel::ActionType::IdType,
          typename PrunerT = NullPruner<std::shared_ptr<NodeType>>>
class GenericSearch : public SearchAlgorithm<StateT, ActionT>, public events::Subject {
public:
	using BaseClass = SearchAlgorithm<StateT, ActionT>;
	using PlanT = typename BaseClass::PlanT;
	using NodePtr = std::shared_ptr<NodeType>;
	
	//! Relevant events
	using NodeOpenEvent = events::NodeOpenEvent<NodeType>;
	using GoalFoundEvent = events::GoalFoundEvent<NodeType>;
	using NodeExpansionEvent = events::NodeExpansionEvent<NodeType>;
	using NodeCreationEvent = events::NodeCreationEvent<NodeType>;

	//! The only allowed constructor requires the user of the algorithm to inject both
	//! (1) the state model to be used in the search
	//! (2) the open list object to be used in the search
	//! (3) the closed list object to be used in the search
	GenericSearch(const StateModel& model, OpenList&& open, ClosedList&& closed) :
		_model(model), _open(std::move(open)), _closed(std::move(closed))
	{}

	virtual ~GenericSearch() {}
	
	// Disallow copy, but allow move
	GenericSearch(const GenericSearch& other) = delete;
	GenericSearch(GenericSearch&& other) = default;
	GenericSearch& operator=(const GenericSearch& rhs) = delete;
	GenericSearch& operator=(GenericSearch&& rhs) = default;

	bool search(const StateT& s, PlanT& solution) override {
		NodePtr n = std::make_shared<NodeType>( s );
		this->notify(NodeCreationEvent(*n));
		_open.insert(n);

		while ( !_open.is_empty() ) {
			NodePtr current = _open.get_next( );
			
			this->notify(NodeOpenEvent(*current));
			
			if (check_goal(current, solution)) return true;

			// close the node before the actual expansion so that children which are identical
			// to 'current' get properly discarded
			_closed.put(current);
			
			this->notify(NodeExpansionEvent(*current));
			
			for ( const auto& a : _model.applicable_actions( current->state ) ) {
				StateT s_a = _model.next( current->state, a );
				NodePtr successor = std::make_shared<NodeType>( std::move(s_a), a, current );
				
				if (_closed.check(successor)) continue; // The node has already been closed
				if (_open.updatable(successor)) continue; // The node is currently on the open list, we update some of its attributes but there's no need to reinsert it.
				
				this->notify(NodeCreationEvent(*successor));
				if (PrunerT::prune(successor)) continue; // Check if we want to prune the node
				_open.insert( successor );
			}
			
		}
		return false;
	}

	//! Backward chaining procedure to recover a plan from a given node
	virtual void retrieve_solution(NodePtr node, PlanT& solution) {
		while (node->has_parent()) {
			solution.push_back(node->action);
			node = node->parent;
		}
		std::reverse( solution.begin(), solution.end() );
	}
	
	//! Convenience method
	bool solve_model(PlanT& solution) { return search( _model.getTask().getInitialState(), solution ); }
	
protected:
	
	virtual bool check_goal(const NodePtr& node, PlanT& solution) {
		if ( _model.goal(node->state)) { // Solution found, we're done
			this->notify(GoalFoundEvent(*node));
			retrieve_solution(node, solution);
			return true;
		}
		return false;
	}
	
	//! The search model
	const StateModel& _model;
	
	//! The open list
	OpenList _open;
	
	//! The closed list
	ClosedList _closed;
	

	//* Some methods mainly for debugging purposes
	bool check_open_list_integrity() const {
		OpenList copy(_open);
		while (!copy.is_empty()) {
			NodePtr node = copy.get_next();
			check_node_correctness(node);
		}
		return true;
	}
	
	bool check_closed_list_integrity() const {
		ClosedList copy(_closed);
		for (auto node:copy) {
			check_node_correctness(node.second);
		}
		return true;
	}
	
	bool check_node_correctness(const NodePtr& node) const {
		if (node->has_parent()) {
			assert(_model.is_applicable(node->parent->state, node->action));
		}
		return true;
	}
};

}
