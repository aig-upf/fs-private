/*
Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cassert>

#include "generic_state.hxx"
#include "relaxed_generic_state.hxx"
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_problem.hxx>
#include <problem_info.hxx>

namespace aptk { namespace core {
	

RelaxedGenericState::~RelaxedGenericState() {
	assert(checkPointerOwnershipIsCorrect());
}

bool RelaxedGenericState::checkPointerOwnershipIsCorrect() const {
	for (const DomainPtr& domain:_domains) {
		if (domain.use_count() != 1) return false;
	}
	return true;
}

//! Applies the given changeset into the current state.
void RelaxedGenericState::accumulate(const Changeset& changeset) {
	for (const auto& elem:changeset.getEffects()) { 
		set(elem.first);
	}
}

std::ostream& RelaxedGenericState::print(std::ostream& os) const {
	return print(os, *(Problem::getCurrentProblem()));
}

std::ostream& RelaxedGenericState::print(std::ostream& os, const Problem& problem) const {
	auto problemInfo = problem.getProblemInfo();
	os << "RelaxedGenericState[";
	for (unsigned i = 0; i < _domains.size(); ++i) { // Iterate through all the sets
		const DomainPtr& vals = _domains.at(i);
		assert(vals->size() != 0);
		
		os << problemInfo->getVariableName(i) << "={";
		for (const auto& objIdx:*vals) { // Iterate through the set elements.
			os << problemInfo->getObjectName(i, objIdx) << ",";
		}
		os << "}, ";
	}
	os << "]";
	return os;
}

} } // namespaces
