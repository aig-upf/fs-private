/*
Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>
Christian Muise <christian.muise@gmail.com>

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

#include <unordered_set>
#include <fs_types.hxx>
#include <applicability/action_managers.hxx>


namespace fs0 {	class ProblemInfo; class MatchTreeActionManager;}

namespace fs0 { namespace language { namespace fstrips { class Formula; class AtomicFormula; } }}
namespace fs = fs0::language::fstrips;


namespace fs0 {


class NodeCreationContext {
public:
	NodeCreationContext(const AtomIndex& tuple_index,
						const std::vector<VariableIdx>& sorted_vars,
						const std::vector<std::unordered_set<AtomIdx>>& rev_app_index,
						std::vector<bool>& seen) :
						_tuple_index( tuple_index ), _sorted_variables(sorted_vars), _rev_app_index(rev_app_index), _seen(seen) {}

	NodeCreationContext(const NodeCreationContext&) = default;

	const AtomIndex&                            _tuple_index;
	const std::vector<VariableIdx>&             _sorted_variables;
	const std::vector<std::unordered_set<AtomIdx>>&    _rev_app_index;

	//! _seen[i] is true iff variable with index 'i' has been processed
	std::vector<bool>&                           _seen;
};

    class BaseNode {
    public:
        typedef BaseNode*   ptr;

    	virtual ~BaseNode() = default;
    	virtual void generate_applicable_items( const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) const = 0;
    	virtual unsigned count() const = 0;
		virtual unsigned count_nodes() const = 0;
		virtual void count_nodes(unsigned& sw, unsigned& leaf, unsigned& empty) const = 0;
        virtual void print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const = 0;

    	static BaseNode::ptr
        create_tree(std::vector<ActionIdx>&& actions, NodeCreationContext& context);

    	static VariableIdx
        get_best_variable( const NodeCreationContext& context );

    	static bool
        action_done( unsigned i, const NodeCreationContext& context );
    };


    class SwitchNode : public BaseNode {
    	VariableIdx _pivot;
    	std::vector<int>           _immediate_items;
    	std::vector<BaseNode *>    _children;
    	BaseNode *                 _default_child;

    public:
    	SwitchNode(const std::vector<ActionIdx>& actions, NodeCreationContext& context);
		~SwitchNode();

    	void generate_applicable_items(const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) const override;

    	unsigned count() const override;
		unsigned count_nodes() const override;
		void count_nodes(unsigned& sw, unsigned& leaf, unsigned& empty) const override;

        void print(std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager) const override;
    };


    class LeafNode : public BaseNode {
    	std::vector<ActionIdx> _applicable_items;
    public:
    	LeafNode(std::vector<ActionIdx>&& actions) : _applicable_items(std::move(actions)) {}
    	void generate_applicable_items( const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) const override;
    	unsigned count() const override { return _applicable_items.size(); }
    	unsigned count_nodes() const override { return 1; }
    	void count_nodes(unsigned& sw, unsigned& leaf, unsigned& empty) const override { ++leaf; }
        void print(std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager) const override;
    };


    class EmptyNode : public BaseNode {
    public:
    	void generate_applicable_items( const State &, const AtomIndex& tuple_index, std::vector<ActionIdx>& ) const override {}
    	unsigned count() const override { return 0; }
    	unsigned count_nodes() const override { return 1; }
    	void count_nodes(unsigned& sw, unsigned& leaf, unsigned& empty) const override { ++empty; }
        void print(std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager) const override;
    };



    //! Match tree data structure from PRP ( https://bitbucket.org/haz/planner-for-relevant-policies )
    //! Ported to FS by Miquel Ramirez, on December 2016

    class MatchTreeActionManager : public NaiveActionManager {
    public:

        friend class SwitchNode;
        friend class LeafNode;
        friend class EmptyNode;

    	MatchTreeActionManager(const std::vector<const GroundAction*>& actions, const std::vector<const fs::Formula*>& state_constraints, const AtomIndex& tuple_idx);
    	virtual ~MatchTreeActionManager() { if (_tree) delete _tree; };
    	MatchTreeActionManager(const MatchTreeActionManager&) = default;

		//! By definition, the match tree whitelist contains all the applicable actions
		bool whitelist_guarantees_applicability() const override { return true; }

		unsigned count() { return _tree->count(); }

		static void check_match_tree_can_be_used(const ProblemInfo& info);


    protected:
		//! The tuple index of the problem
		const AtomIndex& _tuple_idx;

		//!
        BaseNode::ptr   _tree;

	protected:
		std::vector<ActionIdx> compute_whitelist(const State& state) const override;

		//! Returns all the atoms indexes sorted in descending order of appeareance count in action preconditions
		//! (i.e. the one that appears most goes first), breaking ties lexicographically
		std::vector<VariableIdx> sort_variables(const std::vector<unsigned>& variable_relevance) const;
    };

}
