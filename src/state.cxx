
#include <iostream>
#include <cassert>

#include <state.hxx>
#include <problem.hxx>
#include <problem_info.hxx>

namespace fs0 {
	

//! Applies the given changeset into the current state.
void State::accumulate(const std::vector<Atom>& atoms) {
	for (const Atom& fact:atoms) { 
		set(fact);
	}
	updateHash(); // Important to update the hash value after all the changes have been applied!
}

std::ostream& State::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getInfo();
	os << "State";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) {
		os << problemInfo.getVariableName(i) << "=" << problemInfo.getObjectName(i, _values.at(i));
		if (i < _values.size() - 1) os << ", ";
	}
	os << "]";
	return os;
}

} // namespaces
