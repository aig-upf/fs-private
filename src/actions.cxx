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

#include <actions.hxx>
#include <core_problem.hxx>
#include <problem_info.hxx>
#include <limits>

namespace aptk { namespace core {
	
const ActionIdx CoreAction::INVALID_ACTION = std::numeric_limits<unsigned int>::max();
	
std::ostream& CoreAction::print(std::ostream& os) const {
	auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << getName() << "(";
	for(auto obj:_binding) {
		os << problemInfo->getCustomObjectName(obj) << ", ";
	}
	os << ")";
// 	os << "\n\t" << "APPL: " << *_applicabilityFormula;
	return os;
}

} } // namespaces
