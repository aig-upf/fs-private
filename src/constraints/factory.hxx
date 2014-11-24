
#pragma once

#include <memory>
#include <string>
#include <constraints/constraints.hxx>
#include <constraints/alldiff.hxx>
#include <constraints/sum.hxx>

namespace aptk { namespace core {

/**
 * Constraint instantiation.
 */
class ConstraintFactory
{
public:
	static const Constraint::cptr create(const std::string& name, const VariableIdxVector& variables) {
		if (name == "alldiff") {
			return std::make_shared<AlldiffConstraint>(variables);
		} else if (name == "sum") {
			return std::make_shared<SumConstraint>(variables);
		} else {
			throw std::runtime_error("Unknown constraint name: " + name);
		}
	}
};


} } // namespaces

