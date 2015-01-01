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
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_problem.hxx>
#include <problem_info.hxx>

namespace aptk { namespace core {
	

//! Applies the given changeset into the current state.
void GenericState::accumulate(const Changeset& changeset) {
	for (const auto& elem:changeset.getEffects()) { 
		const auto& fact = elem.first;
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

} } // namespaces
