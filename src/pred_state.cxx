
#include <iostream>
#include <cassert>

#include "pred_state.hxx"
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_problem.hxx>
#include <problem_info.hxx>

namespace aptk { namespace core {

const uint8_t PredicativeState::VAL_FALSE = 0;
const uint8_t PredicativeState::VAL_TRUE = 1;
const uint8_t PredicativeState::VAL_BOTH = 2;
	
const ObjectSet PredicativeState::VEC_FALSE = {0};
const ObjectSet PredicativeState::VEC_TRUE = {1} ;
const ObjectSet PredicativeState::VEC_BOTH = {0, 1};

//! Applies the given changeset into the current state.
void PredicativeState::applyChangeset(const Changeset& changeset) {
	for (const auto& elem:changeset.getEffects()) { 
		const auto& fact = elem.first; // Iterate over all the facts in the changeset
		set(fact);
	}
	updateHash(); // Important to update the hash value after all the changes have been applied!
}

std::ostream& PredicativeState::print(std::ostream& os) const {
	return print(os, *(Problem::getCurrentProblem()));
}


std::ostream& PredicativeState::print(std::ostream& os, const Problem& problem) const {
	auto problemInfo = problem.getProblemInfo();
	os << "PredicativeState";
	if (_relaxed) os << "*";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) { // Iterate through all the sets
		const auto val = _values.at(i);
		if (val == VAL_FALSE) continue;
		if (val == VAL_TRUE) {
			os << problemInfo->getVariableName(i);
		} else {
			os << problemInfo->getVariableName(i) << "*";
		}
		os << ", ";
	}
	os << "]";
	return os;
}

} } // namespaces
