
#include <iostream>
#include <cassert>

#include <state.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {
	

//! Applies the given changeset into the current state.
void State::accumulate(const Atom::vctr& atoms) {
	for (const Atom& fact:atoms) { 
		set(fact);
	}
	updateHash(); // Important to update the hash value after all the changes have been applied!
}

std::ostream& State::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << "State";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) { // Iterate through all the sets
		os << problemInfo.getVariableName(i) << "=" << problemInfo.getObjectName(i, _values.at(i)) << ",";
	}
	os << "]";
	return os;
}

} // namespaces
