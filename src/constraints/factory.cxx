

#include <constraints/factory.hxx>
#include <constraints/constraints.hxx>
#include <constraints/alldiff.hxx>
#include <constraints/sum.hxx>
#include <constraints/external_unary_constraint.hxx>
#include <constraints/external_binary_constraint.hxx>

namespace aptk { namespace core {

const ProblemConstraint::cptr ConstraintFactory::create(const std::string& name, const VariableIdxVector& scope) {
	Constraint* constraint;
	if (name == "alldiff") {
		constraint = new AlldiffConstraint(scope.size());
	} else if (name == "sum") {
		constraint = new SumConstraint(scope.size());
	} else {
		throw std::runtime_error("Unknown constraint name: " + name);
	}
	return new ProblemConstraint(constraint, scope);
}

const ProblemConstraint::cptr ConstraintFactory::createExternalConstraint(ApplicableEntity::cptr entity, unsigned procedureIdx) {
	ProblemConstraint::cptr constraint = nullptr;
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

} } // namespaces

