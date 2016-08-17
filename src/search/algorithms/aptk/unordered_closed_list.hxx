/*
Lightweight Automated Planning Toolkit (LAPKT)
Copyright (C) 2015

<contributors>
Miquel Ramirez <miquel.ramirez@gmail.com>
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

#include <memory>
#include <unordered_set>
#include <utility>

namespace lapkt {

// We need to define custom hash and equality functions for the node-pointer type.
// Indeed, we want to define hash and equality of a node as equivalent to hash and equality of
// the state that corresponds to a node.
template <typename NodePtrT>
struct node_hash {
   size_t operator() (const NodePtrT& node) const { return node->state.hash(); }
};

template <typename NodePtrT>
struct node_equal_to {
   size_t operator() (const NodePtrT& n1, const NodePtrT& n2) const { return n1->state == n2->state; }
};

// A simple typedef to improve legibility
template <typename NodePtrT>
using node_unordered_set = std::unordered_set<NodePtrT, node_hash<NodePtrT>, node_equal_to<NodePtrT>>;

// A closed list is now simply an unordered_set of node pointers, providing some shortcut operations
// plus an update 
template <typename NodeType>
class UnorderedMapClosedList : public node_unordered_set<std::shared_ptr<NodeType>>
{
public:
	using NodePtrT = std::shared_ptr<NodeType>;

	virtual ~UnorderedMapClosedList() = default;

	virtual inline void put(NodePtrT node) { this->insert(node); }

	virtual inline void remove(const NodePtrT node) { this->erase(node); }
	
	virtual inline bool check(const NodePtrT node) { return this->find(node) != this->end(); }

	//! Returns a pointer to a node which is identical to the given node and was already in the list,
	//! if such a node exist, or nullptr otherwise
	virtual NodePtrT seek(NodePtrT node) {
		auto it = this->find(node);
		return (it == this->end()) ? nullptr : *it;
	}
};

}
