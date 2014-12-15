
#pragma once

#include <memory>
#include <string>
#include <constraints/problem_constraints.hxx>
#include <constraints/constraints.hxx>
#include <constraints/alldiff.hxx>
#include <constraints/sum.hxx>

namespace aptk { namespace core {

/**
 * Constraint instantiation routines.
 */
class ConstraintFactory
{
public:
	static const ProblemConstraint::cptr create(const std::string& name, const VariableIdxVector& variables) {
		if (name == "alldiff") {
			return std::make_shared<ProblemConstraint>(new AlldiffConstraint(variables), variables);
		} else if (name == "sum") {
			return std::make_shared<ProblemConstraint>(new SumConstraint(variables), variables);
		} else {
			throw std::runtime_error("Unknown constraint name: " + name);
		}
	}
};


} } // namespaces

