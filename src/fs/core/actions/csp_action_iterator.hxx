
#pragma once

#include <fs/core/constraints/gecode/v2/action_schema_csp.hxx>

#include <gecode/driver.hh>

#include <memory>


namespace fs0 {
    class State;
    class LiftedActionID;
}

namespace fs0::gecode::v2 {
    class SymbolExtensionGenerator;
}

namespace fs0::language::fstrips { class Formula; }
namespace fs = fs0::language::fstrips;

namespace fs0::gecode {

class FSGecodeSpace;

//! An iterator that models action schema applicability as an action CSP.
//! The iterator receives an (ordered) set of lifted-action CSP handlers, and upon iteration
//! returns, chainedly, each of the lifted-action IDs that are applicable.
class CSPActionIterator {
protected:
    const std::vector<v2::ActionSchemaCSP>& schema_csps;
    const std::vector<const PartiallyGroundedAction*>& schemas;

    const State& _state;

    std::vector<Gecode::TupleSet> symbol_extensions;

public:
    CSPActionIterator(
            const State& state,
            const std::vector<v2::ActionSchemaCSP>& schema_csps,
            std::vector<Gecode::TupleSet>&& symbol_extensions,
            const std::vector<const PartiallyGroundedAction*>& schemas);

    class Iterator {
        friend class CSPActionIterator;

    public:
        using engine_t = Gecode::DFS<v2::FSGecodeSpace>;

        ~Iterator();

    protected:
        Iterator(
                const State& state,
                const std::vector<v2::ActionSchemaCSP>& schema_csps,
                const std::vector<const PartiallyGroundedAction*>& schemas,
                const std::vector<Gecode::TupleSet>& symbol_extensions,
                unsigned currentIdx);

        const std::vector<v2::ActionSchemaCSP>& schema_csps;
        const std::vector<const PartiallyGroundedAction*>& schemas;
        std::size_t num_schema_csps;

        const State& _state;

        unsigned _current_handler_idx;

        engine_t* _engine;

        v2::FSGecodeSpace* _csp;

        LiftedActionID* _action;

        const std::vector<Gecode::TupleSet>& symbol_extensions;

        void advance();

        //! Returns true iff a new solution has actually been found
        bool next_solution();

    public:
        const Iterator& operator++() {
            advance();
            return *this;
        }
        const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

        const LiftedActionID& operator*() const { return *_action; }

        //! This is not really true... but will work for the purpose of comparing with the end iterator.
        bool operator==(const Iterator &other) const { return _current_handler_idx == other._current_handler_idx; }
        bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
    };

    Iterator begin() const { return Iterator(_state, schema_csps, schemas, symbol_extensions, 0); }
    Iterator end() const { return Iterator(_state, schema_csps, schemas, symbol_extensions, schema_csps.size()); }
};

} // namespaces
