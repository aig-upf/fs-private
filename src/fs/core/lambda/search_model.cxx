#include <utility>


#include <fs/core/lambda/search_model.hxx>

//#include <fs/core/problem_info.hxx>
//#include <fs/core/atom.hxx>

//#include <utility>

namespace lambda {

GroundForwardSearch::GroundForwardSearch(std::shared_ptr<fs::Problem>  problem) : problem_(std::move(problem)) {}


} // namespaces
