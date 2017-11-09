#include <fs/core/search/drivers/sbfws/relevant_atomset.hxx>

namespace fs0 { namespace bfws {


void RelevantAtomSet::update_from_subset(const std::vector<bool>& phi) {
	throw std::runtime_error("Cannot use vector<bool> as a vector of the result of evaluating feature-set F");
}

void RelevantAtomSet::update_from_subset(const std::vector<int>& phi) {
	throw std::runtime_error("Cannot use vector<int> as a vector of the result of evaluating feature-set F");
}

}}
