
#pragma once

#include <fs/core/base.hxx>
#include <fs/core/fs_types.hxx>

#include <unordered_map>
#include <unordered_set>
#include <gecode/int.hh>

namespace fs0 { class State; class Atom; class AtomIndex; }

namespace Gecode { class IntSet; }

namespace fs0 {

class TransitionGraph {
public:
    using PartialExtensionT = std::vector<Gecode::IntArgs>;

protected:
    const AtomIndex& _tuple_index;

    //! _transitions[X] contains a set with all transition allowed
    //! for state variable X
    const AllTransitionGraphsT _transitions;

    // We will iteratively accumulate in 'reachable' the transitive closure
    // of the graph given as by the edge set in `transitions`
    std::vector<std::unordered_map<object_id, std::unordered_set<object_id>>> _reachable;

    //! The set of domains that are allowed, for each state variable and value
    //! in its domain, according to the monotonicity constraints of the problem.
    //! _allowed_domains[X][x] contain the set of reachable values for variable
    //! X once it has taken value x.
    //! If _allowed_domains[X] is the empty map, then there's no monotonicity
    //! constraint placed on variable X.
    std::vector<std::unordered_map<object_id, Gecode::IntSet>> _allowed_domains;




    std::vector<std::unordered_map<object_id, PartialExtensionT>> _partial_extensions;


    //! '_full_extensions[x]' contains the domain of variable x cast as a
    //! (full) Gecode extension
    std::vector<PartialExtensionT> _full_extensions;


public:

    void preprocess_extensions(const AllTransitionGraphsT &transitions);

    TransitionGraph(const AtomIndex& tuple_index, const AllTransitionGraphsT& transitions);

    const std::vector<const Gecode::IntSet*>
    compute_domains(const std::vector<Atom>& changeset) const;

    bool is_active() const;

    bool transition_is_valid(VariableIdx variable, const object_id& val0, const object_id& val1) const;

    bool has_defined_transitions(VariableIdx var) const;

    const std::vector<Gecode::IntArgs>&
    compute_partial_extension(VariableIdx var, const object_id &val) const;

protected:
    std::unordered_map<object_id, Gecode::IntSet>
    preprocess_extension(const std::unordered_map<object_id, std::unordered_set<object_id>>& rechable);

    std::unordered_map<object_id, std::unordered_set<object_id>>
    compute_reachable_sets(const TransitionGraphT &transitions);

    std::unordered_map<object_id, PartialExtensionT>
    precompute_partial_extensions(VariableIdx var,
                                   const std::unordered_map<object_id, std::unordered_set<object_id>> &rechable);

    void precompute_full_extensions();
};

}