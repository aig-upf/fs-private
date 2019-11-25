
#include <utility>


#include <fs/core/lambda/search/models.hxx>


namespace lambda::search {

GroundForwardSearch::GroundForwardSearch(std::shared_ptr<fs::Problem> problem) : problem_(std::move(problem)) {}

bool GroundForwardSearch::goal(const State &state) const {
    return false;
}

const State& GroundForwardSearch::init() const {
    return *init_;
}

std::shared_ptr<State> GroundForwardSearch::next(const State &state, const ActionId &a) const {
    return nullptr;
}

std::shared_ptr<ApplicableActionSet> GroundForwardSearch::applicable_actions(const State &state) const {
    return nullptr;
}


} // namespaces
