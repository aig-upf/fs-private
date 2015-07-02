
#include <iostream>
#include <cassert>

#include <relaxed_generic_state.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {
	

RelaxedGenericState::~RelaxedGenericState() {
	assert(checkPointerOwnershipIsCorrect());
}

bool RelaxedGenericState::checkPointerOwnershipIsCorrect() const {
	for (const DomainPtr& domain:_domains) {
		if (domain.use_count() != 1) return false;
	}
	return true;
}

std::ostream& RelaxedGenericState::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << "RelaxedGenericState[";
	for (unsigned i = 0; i < _domains.size(); ++i) { // Iterate through all the sets
		const DomainPtr& vals = _domains.at(i);
		assert(vals->size() != 0);
		
		os << problemInfo.getVariableName(i) << "={";
		for (const auto& objIdx:*vals) { // Iterate through the set elements.
			os << problemInfo.getObjectName(i, objIdx) << ",";
		}
		os << "}, ";
	}
	os << "]";
	return os;
}

} // namespaces
