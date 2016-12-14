/*
Lightweight Automated Planning Toolkit
Copyright (C) 2015

<contributors>
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
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
*/

#pragma once

#include <vector>
#include <cassert>
#include <functional>
#include <queue>
#include <memory>
#include <unordered_set>

#include <aptk2/search/interfaces/open_list.hxx>

namespace lapkt {

//! We need to define custom hash, equality and comparison functions for the node-pointer type.

template <typename NodePtrT>
struct node_comparer { bool operator()(const NodePtrT& n1, const NodePtrT& n2) const { return *n1 > *n2; } };

template <typename NodePtrT>
struct node_hash { size_t operator() (const NodePtrT& node) const { return node->state.hash(); } };

template <typename NodePtrT>
struct node_equal_to { bool operator() (const NodePtrT& n1, const NodePtrT& n2) const { return n1->state == n2->state; } };


template <typename NodeType,
          typename Heuristic,
          typename NodePtrT = std::shared_ptr<NodeType>,
          typename Container = std::vector<NodePtrT>,
          typename Comparer = node_comparer<NodePtrT>
>
class StlSortedOpenList : public aptk::OpenList<NodeType, std::priority_queue<NodePtrT, Container, Comparer>>
{
public:
	//! The constructor of a sorted open list needs to specify the heuristic to sort the nodes with
	StlSortedOpenList(Heuristic& heuristic)
		: _heuristic(heuristic), already_in_open_()
	{}
	virtual ~StlSortedOpenList() = default;
	
	// Disallow copy, but allow move
	StlSortedOpenList(const StlSortedOpenList& other) = default;
	StlSortedOpenList(StlSortedOpenList&& other) = default;
	StlSortedOpenList& operator=(const StlSortedOpenList& rhs) = default;
	StlSortedOpenList& operator=(StlSortedOpenList&& rhs) = default;

	bool insert(const NodePtrT& node) override {
		if ( node->dead_end() ) return false;
		this->push( node );
		already_in_open_.insert( node );
		return true;
	}

	//! Check if the open list already contains a node 'previous' referring to the same state.
	//! If that is the case, there'll be no need to reinsert the node, and we signal so returning true.
	//! If, in addition, 'previous' had a higher g-value, we do an in-place modification of it.
	bool updatable(const NodePtrT& node) {
		auto it = already_in_open_.find(node);
		if (it == already_in_open_.end()) return false; // No node with the same state is in the open list
		
		// Else the node was already in the open list and we might want to update it
		
		// @TODO: Very important: updating g is correct provided that
		// the order of the nodes in the open list is not changed by
		// updating it. This is an open problem with the design,
		// and a more definitive solution needs to be found (soon).
		(*it)->update_in_open_list(node);
		
		return true;
	}

	NodePtrT get_next() override {
		assert( !is_empty() );
		NodePtrT node = this->top();
		this->pop();
		already_in_open_.erase(node);
		return node;
	}

	bool is_empty() const override { return this->empty(); }
	
protected:
	//! The heuristic we'll use to sort the nodes
	Heuristic& _heuristic;
	
	//! An index of the nodes with are in the open list at any moment, for faster access
	using node_unordered_set = std::unordered_set<NodePtrT, node_hash<NodePtrT>, node_equal_to<NodePtrT>>;
	node_unordered_set already_in_open_;
};


}
