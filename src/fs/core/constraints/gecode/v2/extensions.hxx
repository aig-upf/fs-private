
#pragma once

#include <fs/core/base.hxx>

#include <gecode/int.hh>


namespace fs0 {
    class AtomIndex;
    class ProblemInfo;
    class State;
}

namespace fs0::gecode::v2 {

class IndividualSymbolExtensionGenerator;

//! A SymbolExtensionGenerator is in charge of generating the extension (in the form of a Gecode::TupleSet of a given
//! symbol, function or predicate. The main challenge for this is that there might be symbols that are only "partially
//! fluent", i.e. some of its points have been determined to be static, e.g. through reachability analysis, and hence
//! are not a state variable and are not part of the state; whereas some of its points are indeed state variables.
//! To deal with this, the SymbolExtensionGenerator precompiles for each symbol P a list of tuples of ints
//! that are statically part of the extension of P in any state, and precompiles also a mapping between fluent atoms
//! (i.e. "pairs of state-variable/value") and the tuple of ints that will be part of the extension of P whenever that
//! atom is part of the state. Then, to generate the extension of P on state s, we start with the static tuples, then
//! iterate over all atoms, and if they are part of the state, add the corresponding tuple to the extension.
//!
//! Note that function extensions include the function codomain value, i.e. contain tuples of size equal to the arity
//! of the function plus one.
class SymbolExtensionGenerator {
public:
    //! `managed[i]` denotes that we want to manage symbol with ID i.
    SymbolExtensionGenerator(const ProblemInfo& info, const AtomIndex& tuple_index, std::vector<unsigned> managed);

    SymbolExtensionGenerator(const SymbolExtensionGenerator&) = default;
    SymbolExtensionGenerator(SymbolExtensionGenerator&&) = default;
    SymbolExtensionGenerator& operator=(const SymbolExtensionGenerator&) = delete;
    SymbolExtensionGenerator& operator=(SymbolExtensionGenerator&&) = delete;

    std::vector<Gecode::TupleSet> instantiate(const State& state) const;

    Gecode::TupleSet retrieve_static_tupleset(unsigned symbol_id) const;

    //! Return whether the given symbol has no fluent tuples, meaning it can be evaluated statically
    bool is_fully_static(unsigned symbol_id) const;

protected:
    std::vector<unsigned> managed;

    std::vector<IndividualSymbolExtensionGenerator> individuals;
};

//! While the above SymbolExtensionGenerator class takes care of all symbols in the problem, this class just takes
//! care of one single symbol.
class IndividualSymbolExtensionGenerator {
public:
    IndividualSymbolExtensionGenerator(unsigned symbol_id, const ProblemInfo& info, const AtomIndex& tuple_index);

    IndividualSymbolExtensionGenerator(const IndividualSymbolExtensionGenerator&) = default;
    IndividualSymbolExtensionGenerator(IndividualSymbolExtensionGenerator&&) = default;
    IndividualSymbolExtensionGenerator& operator=(const IndividualSymbolExtensionGenerator&) = delete;
    IndividualSymbolExtensionGenerator& operator=(IndividualSymbolExtensionGenerator&&) = delete;

    Gecode::TupleSet instantiate(const State& state) const;

    Gecode::TupleSet retrieve_static_tupleset() const;

protected:
    friend class SymbolExtensionGenerator;
    //! A private constructor to emulate null-like values
    IndividualSymbolExtensionGenerator();

    unsigned symbol_id;
    unsigned arity;

    std::vector<Gecode::IntArgs> static_tuples;

    //! A triplet <x, y, z> denotes that when s[x]=y, we need to add (tuple_t) z to the extension
    std::vector<std::tuple<unsigned, object_id, Gecode::IntArgs>> fluent_tuples;
};

} // namespaces
