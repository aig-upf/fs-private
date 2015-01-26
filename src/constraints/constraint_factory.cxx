

#include <constraints/constraint_factory.hxx>
#include <constraints/scoped_alldiff_constraint.hxx>
#include <constraints/scoped_sum_constraint.hxx>

namespace fs0 {

const ScopedConstraint::cptr ConstraintFactory::create(const std::string& name, const ObjectIdxVector& parameters, const VariableIdxVector& scope) {
	if (name == "alldiff") {
		return new ScopedAlldiffConstraint(scope);
	} else if (name == "sum") {
		return new ScopedSumConstraint(scope);
	} else {
		throw std::runtime_error("Unknown constraint name: " + name);
	}
}

/*
const ScopedConstraint::cptr ConstraintFactory::createExternalConstraint(ApplicableEntity::cptr entity, unsigned procedureIdx) {
	const VariableIdxVector& relevant = entity->getApplicabilityRelevantVars(procedureIdx);
	if (relevant.size() == 1) {
		constraint = new ExternalUnaryConstraint(entity, procedureIdx);
	} else if (relevant.size() == 2) {
		constraint = new ExternalBinaryConstraint(entity, procedureIdx);
	} else {
		throw std::runtime_error("Shouldn't have n-ary constraints here");
	}
	return constraint;
}
*/
} // namespaces

