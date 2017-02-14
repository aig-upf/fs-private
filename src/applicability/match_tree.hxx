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

#include <fs_types.hxx>
#include <applicability/action_managers.hxx>
#include <boost/concept_check.hpp>


namespace fs0 {	class ProblemInfo; class MatchTreeActionManager;}

namespace fs0 { namespace language { namespace fstrips { class Formula; class AtomicFormula; } }}
namespace fs = fs0::language::fstrips;


namespace fs0 {


class NodeCreationContext {
public:
	NodeCreationContext(std::vector<ActionIdx>& actions,
						const AtomIndex& tuple_index,
						const std::vector<unsigned>& sorted_atoms,
						const std::vector<std::vector<AtomIdx>>& rev_app_index,
						std::vector<bool>& seen) : 
						_actions( actions ), _tuple_index( tuple_index ), _sorted_atoms(sorted_atoms), _rev_app_index(rev_app_index), _seen(seen) {}

	NodeCreationContext(const NodeCreationContext& other, std::vector<ActionIdx>& actions) : 
		_actions(actions), _tuple_index(other._tuple_index), _sorted_atoms(other._sorted_atoms), _rev_app_index(other._rev_app_index), _seen(other._seen)
	{}
						
	std::vector<ActionIdx>&                     _actions;
	const AtomIndex&                            _tuple_index;
	const std::vector<unsigned>&                _sorted_atoms;
	const std::vector<std::vector<AtomIdx>>&    _rev_app_index;

	//! _seen[i] is true iff  the tuple with index i has already been "seen"
	std::vector<bool>&                           _seen;
};

    class BaseNode {
    public:
        typedef BaseNode*   ptr;

    	virtual ~BaseNode() = default;
    	virtual void generate_applicable_items( const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) = 0;
    	virtual int count() const = 0;
        virtual void print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const = 0;

    	static BaseNode::ptr
        create_tree(  NodeCreationContext& context );

    	static AtomIdx
        get_best_atom( const NodeCreationContext& context );

    	static bool
        action_done( unsigned i, const NodeCreationContext& context );
    };


    class SwitchNode : public BaseNode {
    	AtomIdx _pivot;
    	std::vector<int>           _immediate_items;
    	std::vector<BaseNode *>    _children;
    	BaseNode *                 _default_child;

    public:
    	SwitchNode( NodeCreationContext& context );
		~SwitchNode();

    	virtual void generate_applicable_items(     const State& s,
                                                    const AtomIndex& tuple_index,
                                                    std::vector<ActionIdx>& actions ) override;

    	virtual int    count() const;
        virtual void print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const;
    };


    class LeafNode : public BaseNode {
    	std::vector<ActionIdx> _applicable_items;
    public:
    	LeafNode(std::vector<ActionIdx>& actions) : _applicable_items() { _applicable_items.swap(actions); }
    	virtual void generate_applicable_items( const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) override;
    	virtual int count() const { return _applicable_items.size(); }
        virtual void print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const;
    };


    class EmptyNode : public BaseNode {
    public:
    	virtual void generate_applicable_items( const State &, const AtomIndex& tuple_index, std::vector<ActionIdx>& ) override {}
    	virtual int count() const { return 0; }
        virtual void print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const;
    };



    //! Match tree data structure from PRP ( https://bitbucket.org/haz/planner-for-relevant-policies )
    //! Ported to FS by Miquel Ramirez, on December 2016

    class MatchTreeActionManager : public NaiveActionManager {
    public:

        friend class SwitchNode;
        friend class LeafNode;
        friend class EmptyNode;

    	MatchTreeActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints, const AtomIndex& tuple_idx);
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
		std::vector<unsigned> sort_atom_idxs(const std::vector<std::vector<ActionIdx>>& applicability_idx) const;
    };

}
