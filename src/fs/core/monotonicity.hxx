
#pragma once

#include <fs/core/base.hxx>
#include <fs/core/fs_types.hxx>

#include <unordered_map>
#include <unordered_set>

namespace fs0 { class State; class Atom; }

namespace Gecode { class IntSet; }

namespace fs0 {

class TransitionGraph {
public:


protected:
    //! _transitions[X] contains a set with all transition allowed
    //! for state variable X
    const AllTransitionGraphsT _transitions;


    //! The set of domains that are allowed, for each state variable and value
    //! in its domain, according to the monotonicity constraints of the problem.
    //! _allowed_domains[X][x] contain the set of reachable values for variable
    //! X once it has taken value x.
    //! If _allowed_domains[X] is the empty map, then there's no monotonicity
    //! constraint placed on variable X.
    std::vector<std::unordered_map<object_id, Gecode::IntSet>> _allowed_domains;

public:

    TransitionGraph(const AllTransitionGraphsT& transitions) :
        _transitions(transitions),
        _allowed_domains(build(transitions))
    {}

    const std::vector<const Gecode::IntSet*>
    compute_domains(const std::vector<Atom>& changeset) const;




    bool transition_is_valid(VariableIdx variable, const object_id& val0, const object_id& val1) const;

protected:
    static std::vector<std::unordered_map<object_id, Gecode::IntSet>>
    build(const AllTransitionGraphsT& transitions);

    static std::unordered_map<object_id, Gecode::IntSet>
    build_single(const TransitionGraphT& set);
};

}