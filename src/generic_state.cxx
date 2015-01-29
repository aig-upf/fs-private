
#include <iostream>
#include <cassert>

#include "generic_state.hxx"
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {
	

//! Applies the given changeset into the current state.
void GenericState::accumulate(const Fact::vctr& atoms) {
	for (const Fact& fact:atoms) { 
		set(fact);
	}
	updateHash(); // Important to update the hash value after all the changes have been applied!
}

std::ostream& GenericState::print(std::ostream& os) const {
	return print(os, *(Problem::getCurrentProblem()));
}

std::ostream& GenericState::print(std::ostream& os, const Problem& problem) const {
	auto problemInfo = problem.getProblemInfo();
	os << "GenericState";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) { // Iterate through all the sets
		os << problemInfo->getVariableName(i) << "=" << problemInfo->getObjectName(i, _values.at(i)) << ",";
	}
	os << "]";
	return os;
}

} // namespaces
