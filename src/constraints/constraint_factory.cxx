

#include <constraints/constraint_factory.hxx>
#include <constraints/direct/builtin.hxx>
#include <constraints/direct/alldiff_constraint.hxx>
#include <constraints/direct/sum_constraint.hxx>

namespace fs0 {

//! @deprecated ??
const DirectConstraint::cptr ConstraintFactory::create(const std::string& name, const ObjectIdxVector& parameters, const VariableIdxVector& scope) {

	if (name == "geq") {
		throw std::runtime_error("Use a <= constraint with a reversed scope");
	} else if (name == "alldiff") {
		return new AlldiffConstraint(scope);
	} else if (name == "sum") {
		return new SumConstraint(scope);
	} else {
		throw std::runtime_error("Unknown constraint name: " + name);
	}
}


} // namespaces

