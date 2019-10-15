
#pragma once

#include <memory>
#include <vector>

#include <fs/core/utils/sdd.hxx>


namespace fs0 {
	class State;
	class LiftedActionID;
}

namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

    class AtomIndex;

    //! An iterator that models action schema applicability as an action CSP.
    //! The iterator receives an (ordered) set of lifted-action CSP handlers, and upon iteration
    //! returns, chainedly, each of the lifted-action IDs that are applicable.
    class SDDActionIterator {
    protected:
        const State& state_;

        const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds_;

    public:
        SDDActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, const AtomIndex& tuple_index);

        class Iterator {
            friend class SDDActionIterator;

        public:

            ~Iterator();

        protected:
            Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds, unsigned currentIdx);

            const State& state_;

            const std::vector<std::shared_ptr<ActionSchemaSDD>>& sdds_;

            unsigned current_sdd_idx_;

            SddNode* current_sdd_;

            SDDModelEnumerator* model_iterator_;

            LiftedActionID* _action;

            //! Advance into the next SDD model
            void advance();

        public:
            const Iterator& operator++() {
                advance();
                return *this;
            }
            const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

            const LiftedActionID& operator*() const { return *_action; }

            //! This is not really true... but will work for the purpose of comparing with the end iterator.
            bool operator==(const Iterator &other) const { return current_sdd_idx_ == other.current_sdd_idx_; }
            bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
        };

        Iterator begin() const { return {state_, sdds_, 0}; }
        Iterator end() const { return {state_, sdds_, (unsigned int) sdds_.size()}; }
    };


} // namespaces

