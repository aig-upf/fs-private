
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
	const ProblemInfo& info = Problem::getInfo();
	os << "State";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) {
		if (info.getVariableGenericType(i) == ProblemInfo::ObjectType::BOOL) {
			if (_values.at(i) == 0) continue;
			
			// Print only those atoms which are true in the state
			os << info.getVariableName(i);
		} else {
			os << info.getVariableName(i) << "=" << info.getObjectName(i, _values.at(i));
		}
		if (i < _values.size() - 1) os << ", ";
	}
	os << "]";
	return os;
}

} // namespaces
