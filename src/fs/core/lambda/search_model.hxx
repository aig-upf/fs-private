
#pragma once

//#include <fs/core/fs_types.hxx>
// #include <fs/core/utils/bitsets.hxx>

#include <vector>
#include <ostream>
#include <memory>

#include <fs/core/lambda/states.hxx>
#include <fs/core/fstrips/problem.hxx>

namespace fs = fs0::fstrips;

namespace lambda {

class State;

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

class GroundForwardSearch {
public:
    using state_t = State;
    using action_id_t = unsigned;

    explicit GroundForwardSearch(std::shared_ptr<fs::Problem> problem);
    ~GroundForwardSearch() = default;

    GroundForwardSearch(const GroundForwardSearch&) = default;
    GroundForwardSearch& operator=(const GroundForwardSearch&) = default;
    GroundForwardSearch(GroundForwardSearch&&) = default;
    GroundForwardSearch& operator=(GroundForwardSearch&&) = default;

    //! Return initial state of the problem
    const State& init() const;

    //! Return true if state is a goal state
    bool goal(const State& state) const;

    //! Return the state resulting from applying the given action action on the given state
    State next(const State& state, const action_id_t id) const;

    //! Return set of actions applicable in the given state
//    GroundApplicableSet applicable_actions(const State& state) const;

    //! Return the problem this search model refers to
    const fs::Problem& problem() const { return *problem_; }

    //    static ActionManagerI* build_action_manager(const Problem& problem);

protected:
    // The underlying planning problem.
    std::shared_ptr<fs::Problem> problem_;

//    std::unique_ptr<ActionManagerI> _manager;

    std::vector<unsigned> actions_;
};

} // namespaces
