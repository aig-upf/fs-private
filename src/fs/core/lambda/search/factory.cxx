

#include <fs/core/lambda/search/factory.hxx>

namespace lambda::search {

std::shared_ptr<SearchAlgorithm>
lambda::search::SearchFactory::breadth_first_search(const std::shared_ptr<SearchModel> &model) {
    return nullptr;
}

std::shared_ptr<SearchAlgorithm>
lambda::search::SearchFactory::iw(const std::shared_ptr<SearchModel> &model) {
    return nullptr;
}

std::shared_ptr<SearchAlgorithm>
lambda::search::SearchFactory::bfws(const std::shared_ptr<SearchModel> &model) {
    return nullptr;
}

}