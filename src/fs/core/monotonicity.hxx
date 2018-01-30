
#pragma once

#include <fs/core/base.hxx>
#include <fs/core/fs_types.hxx>

#include <boost/dynamic_bitset.hpp>

#include <unordered_map>
#include <unordered_set>
#include <gecode/int.hh>

namespace fs0 { class State; class Atom; class AtomIndex; }

namespace Gecode { class IntSet; }

namespace fs0 {

class TransitionGraph {
public:
    using PartialExtensionT = std::vector<Gecode::IntArgs>;
//    using BitmapT = std::vector<bool>;
    using BitmapT = boost::dynamic_bitset<>;

    //! Return a vector V of domains (i.e. a vector of bitsets) such that V[i]
    //! contains the set of reachable values for _the new value taken by the i-th variable_
    //! in the order given in the changeset (which is NOT equal to variable with index "i"!)
    std::vector<TransitionGraph::BitmapT>
    retrieve_domains(const std::vector<Atom>& changeset) const;

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


    std::vector<std::unordered_map<object_id, BitmapT>> _reachable_bitsets;





    std::vector<std::unordered_map<object_id, PartialExtensionT>> _partial_extensions;


    //! '_full_extensions[x]' contains the domain of variable x cast as a
    //! (full) Gecode extension
    std::vector<PartialExtensionT> _full_extensions;


    //! The indexes of all variables that are considered monotonic
    std::unordered_set<VariableIdx> _monotonic_variables;


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

    const std::unordered_set<VariableIdx>& monotonic_variables() const {
        return _monotonic_variables; }

    bool is_monotonic(VariableIdx var) const {
        return _monotonic_variables.find(var) != _monotonic_variables.end();
    }

protected:
    std::unordered_map<object_id, Gecode::IntSet>
    preprocess_extension(const std::unordered_map<object_id, std::unordered_set<object_id>>& rechable);

    std::unordered_map<object_id, std::unordered_set<object_id>>
    compute_reachable_sets(const TransitionGraphT &transitions);

    std::unordered_map<object_id, PartialExtensionT>
    precompute_partial_extensions(VariableIdx var,
                                   const std::unordered_map<object_id, std::unordered_set<object_id>> &rechable);

    void precompute_full_extensions();

    std::unordered_map<object_id, BitmapT> generate_bitset(int max_value,
                                                           const std::unordered_map<object_id, std::unordered_set<object_id>>& rechable);
};


//!
class DomainTracker {
protected:
    //! _reachable[x] contains a bitmap with the set of reachable values for state variable x
    //! in a certain state of the search. If variable x is not a monotonic variable, then
    //! _reachable[x] will be empty
    std::unique_ptr<std::vector<TransitionGraph::BitmapT>> _domains;
public:
    const std::vector<TransitionGraph::BitmapT>& domains() const { return *_domains; }

    DomainTracker() :
            _domains(new std::vector<TransitionGraph::BitmapT>)
    {}

    DomainTracker(std::vector<TransitionGraph::BitmapT>&& domains) :
            _domains(new std::vector<TransitionGraph::BitmapT>(std::move(domains)))
    {}

    std::size_t size() const { assert(_domains); return _domains->size(); }
    bool is_null() const { assert(_domains); return _domains->empty(); }

    void release() {
//        _domains.clear();
//        std::vector<TransitionGraph::BitmapT>().swap(_domains);
        _domains.reset();
    }

    std::vector<Gecode::IntSet> to_intsets() const;
};


namespace print {

class bitset {
protected:
    const VariableIdx _var;
    const TransitionGraph::BitmapT& _bitset;

public:
    bitset(VariableIdx var, const TransitionGraph::BitmapT& bs) :
            _var(var), _bitset(bs) {}

    friend std::ostream& operator<<(std::ostream &os, const bitset& o) { return o.print(os); }
    std::ostream& print(std::ostream& os) const;
};


class domain_tracker {
protected:
    const DomainTracker& _domains;

public:
    domain_tracker(const DomainTracker& domains) : _domains(domains) {}

    friend std::ostream& operator<<(std::ostream &os, const domain_tracker& o) { return o.print(os); }
    std::ostream& print(std::ostream& os) const;
};

}

} //namespaces

