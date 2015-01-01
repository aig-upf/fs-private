

#include <constraints/factory.hxx>
#include <constraints/constraints.hxx>
#include <constraints/alldiff.hxx>
#include <constraints/sum.hxx>
#include <constraints/external_unary_constraint.hxx>
#include <constraints/external_binary_constraint.hxx>

namespace aptk { namespace core {

const ProblemConstraint::cptr ConstraintFactory::create(const std::string& name, const VariableIdxVector& scope) {
	if (name == "alldiff") {
		return std::make_shared<ProblemConstraint>(new AlldiffConstraint(scope.size()), scope);
	} else if (name == "sum") {
		return std::make_shared<ProblemConstraint>(new SumConstraint(scope.size()), scope);
	} else {
		throw std::runtime_error("Unknown constraint name: " + name);
	}
}

const ProblemConstraint::cptr ConstraintFactory::createExternalConstraint(ApplicableEntity::cptr entity, unsigned procedureIdx) {
	ProblemConstraint::cptr constraint = nullptr;
	const VariableIdxVector& relevant = entity->getApplicabilityRelevantVars(procedureIdx);
	if (relevant.size() == 1) {
		constraint = std::make_shared<ExternalUnaryConstraint>(entity, procedureIdx);
	} else if (relevant.size() == 2) {
		constraint = std::make_shared<ExternalBinaryConstraint>(entity, procedureIdx);
	} else {
		throw std::runtime_error("Shouldn't have n-ary constraints here");
	}
	return constraint;
}

} } // namespaces

