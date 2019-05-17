
#pragma once

#include <vector>
#include <memory>

#include <fs/core/search/stats.hxx>
#include <fs/core/utils/system.hxx>


namespace lambda::search {

class State;
class Plan;
class SearchModel;

class SearchAlgorithm {
public:
    virtual ~SearchAlgorithm() = delete;

    virtual fs0::ExitCode search(const State& s, Plan& solution) = 0;
};

//! A factory of possible search engines
class SearchFactory {
public:
    static std::shared_ptr<SearchAlgorithm> breadth_first_search(const std::shared_ptr<SearchModel>& model);

    static std::shared_ptr<SearchAlgorithm> iw(const std::shared_ptr<SearchModel>& model);

    static std::shared_ptr<SearchAlgorithm> bfws(const std::shared_ptr<SearchModel>& model);
};

} // namespaces
