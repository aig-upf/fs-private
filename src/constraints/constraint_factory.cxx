

#include <constraints/constraint_factory.hxx>
#include <constraints/scoped_alldiff_constraint.hxx>
#include <constraints/scoped_sum_constraint.hxx>
#include <constraints/builtin_constraints.hxx>

namespace fs0 {

const ScopedConstraint::cptr ConstraintFactory::create(const std::string& name, const ObjectIdxVector& parameters, const VariableIdxVector& scope) {

	if (name == "geq") {
		throw std::runtime_error("Use a <= constraint with a reversed scope");
	} else if (name == "alldiff") {
		return new ScopedAlldiffConstraint(scope);
	} else if (name == "sum") {
		return new ScopedSumConstraint(scope);
	} else {
		throw std::runtime_error("Unknown constraint name: " + name);
	}
}


} // namespaces

