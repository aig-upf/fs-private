
#pragma once

#include <vector>
#include <ostream>
#include <memory>

namespace lambda::search {

class State;
class ActionId;

class ApplicableActionSet {
public:
    virtual ~ApplicableActionSet() = default;
};

/* TODO: Work in progress
//! A simple iterator strategy to iterate over the actions applicable in a given state.
class NaiveIterationApplicableSet {
protected:
    friend class NaiveActionManager;

    const State& state_;

    const ActionGrounding_& grounding;

    const ActionManagerI& _manager;

    NaiveIterationApplicableSet(const State& state, const ActionGrounding& grounding, const ActionManagerI& manager):
            state_(state), grounding_(grounding), _manager(manager), _whitelist(action_whitelist)
    {}

    class Iterator {

        const State& state_;

        const ActionManagerI& _manager;

        unsigned _index;

        void advance() {
            const std::vector<const GroundAction*>& actions = _manager.getAllActions();

            for (unsigned sz = _whitelist.size();_index < sz; ++_index) {
                unsigned action_idx = _whitelist[_index];

                if (_manager.applicable(state_, *actions[action_idx])) { // The action is applicable, break the for loop.
                    return;
                }
            }

        }

    public:
        Iterator(const State& state, const ActionManagerI& manager, unsigned index) :
                _manager(manager),
                state_(state),
                _index(index)
        {
            advance();
        }

        const Iterator& operator++() {
            ++_index;
            advance();
            return *this;
        }

        const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

        ActionId operator*() const { return _whitelist[_index]; }

        bool operator==(const Iterator &other) const { return _index == other._index; }
        bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
    };

public:
    Iterator begin() const { return Iterator(state_, _manager, 0); }
    Iterator end() const { return Iterator(state_, _manager); }
};

*/

//! A common base class for different (applicable) action enumeration strategies that one might use,
//! both grounded and lifted.
class ActionEnumeratorI {
public:
    virtual ~ActionEnumeratorI() = default;

    //! Return the set of all actions applicable in a given state
    virtual std::shared_ptr<ApplicableActionSet> applicable(const State& state) const = 0;
};

//! An action enumeration strategy that simply iterates over all possible action groundings and checks their
//! applicability on the given state one by one.
class NaiveActionEnumerator : public ActionEnumeratorI {
public:
    ~NaiveActionEnumerator() override = default;

    //! Return the set of all actions applicable in a given state
    std::shared_ptr<ApplicableActionSet> applicable(const State& state) const override;
};


} // namespaces
