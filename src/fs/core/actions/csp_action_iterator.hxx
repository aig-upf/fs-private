
#pragma once

#include <memory>

#include <gecode/driver.hh>
#include <fs/core/constraints/gecode/extensions.hxx>

namespace fs0 {
    class State;
    class LiftedActionID;
}

namespace fs0::language::fstrips { class Formula; }
namespace fs = fs0::language::fstrips;

namespace fs0::gecode {

class GecodeSpace;
class LiftedActionCSP;

//! An iterator that models action schema applicability as an action CSP.
//! The iterator receives an (ordered) set of lifted-action CSP handlers, and upon iteration
//! returns, chainedly, each of the lifted-action IDs that are applicable.
class CSPActionIterator {
protected:
    const std::vector<std::shared_ptr<LiftedActionCSP>>& _handlers;

    const State& _state;

    const std::vector<const fs::Formula*>& _state_constraints;

    StateBasedExtensionHandler _extension_handler;

public:
    CSPActionIterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const std::vector<const fs::Formula*>& state_constraints, const AtomIndex& tuple_index);

    class Iterator {
        friend class CSPActionIterator;

    public:
        using engine_t = Gecode::DFS<GecodeSpace>;

        ~Iterator();

    protected:
        Iterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const std::vector<const fs::Formula*>& state_constraints, const StateBasedExtensionHandler& extension_handler, unsigned currentIdx);

        const std::vector<std::shared_ptr<LiftedActionCSP>>& _handlers;

        const State& _state;

        unsigned _current_handler_idx;

        engine_t* _engine;

        GecodeSpace* _csp;

        LiftedActionID* _action;

        //! The state constraints
        const std::vector<const fs::Formula*>& _state_constraints;

        const StateBasedExtensionHandler& _extension_handler;

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

    Iterator begin() const { return Iterator(_state, _handlers, _state_constraints, _extension_handler, 0); }
    Iterator end() const { return Iterator(_state,_handlers, _state_constraints, _extension_handler, _handlers.size()); }
};

} // namespaces
