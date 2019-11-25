
#pragma once

#include <vector>
#include <ostream>
#include <memory>

#include <fs/core/lambda/states.hxx>
#include <fs/core/fstrips/problem.hxx>

namespace fs = fs0::fstrips;

namespace lambda::search {

class State;
class ApplicableActionSet;

/*
class SimpleAction {
public:

    const std::vector<bool> preconditions;
    const std::vector<bool> adds;
    const std::vector<bool> deletes;
};

bool applicable(states::BinaryState s, SimpleAction a) {
    // An action is applicable if forall i prec[i] => s[i]
//    return s.data() ;
    assert(false);
    return false;
}

states::BinaryState next(states::BinaryState s, SimpleAction a) {
    return states::BinaryState(s.data());
}
*/

class ActionId {
public:
    virtual ~ActionId() = default;
};

class Plan {
public:
    virtual ~Plan() = delete;
};

//! An abstract base class for search models
class SearchModel {
public:
    virtual ~SearchModel() = default;

    //! Returns initial state of the problem
    virtual const State &init() const = 0;

    //! Returns true if state is a goal state
    virtual bool goal(const State &state) const = 0;

    //! Returns applicable action set object
    virtual std::shared_ptr<ApplicableActionSet> applicable_actions(const State &state) const = 0;

//    virtual bool is_applicable(const State &state, const ActionId &action) const = 0;

    //! Returns the state resulting from applying the given action action on the given state
    virtual std::shared_ptr<State> next(const State &state, const ActionId &a) const = 0;

};


class GroundForwardSearch : public SearchModel {
public:
//    using state_t = State;
//    using action_id_t = unsigned;

    explicit GroundForwardSearch(std::shared_ptr<fs::Problem> problem);
    ~GroundForwardSearch() override = default;

    GroundForwardSearch(const GroundForwardSearch &) = default;
    GroundForwardSearch(GroundForwardSearch &&) = default;
    GroundForwardSearch &operator=(const GroundForwardSearch &) = default;
    GroundForwardSearch &operator=(GroundForwardSearch &&) = default;

    //! Return initial state of the problem
    const State& init() const override;

    //! Return true if state is a goal state
    bool goal(const State &state) const override;

    //! Return the state resulting from applying the given action action on the given state
    std::shared_ptr<State> next(const State &state, const ActionId &a) const override;

    //! Return set of actions applicable in the given state
    std::shared_ptr<ApplicableActionSet> applicable_actions(const State &state) const override;


protected:
    // The underlying planning problem.
    std::shared_ptr<fs::Problem> problem_;

//    std::unique_ptr<ActionManagerI> _manager;

    std::vector<unsigned> actions_; // ground actions?

    const std::shared_ptr<State> init_;
};


} // namespaces
