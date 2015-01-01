

#include <constraints/external_constraint.hxx>
#include <utils/projections.hxx>

namespace aptk { namespace core {

ExternalConstraint::ExternalConstraint(ApplicableEntity::cptr entity, unsigned procedure_idx) :
	ProblemConstraint(nullptr, entity->getApplicabilityRelevantVars(procedure_idx)), _entity(entity), _procedure_idx(procedure_idx)
{}


bool ExternalConstraint::isSatisfied(const State& s) const {
	ObjectIdxVector projection = Projections::project(s, _scope);
	return _entity->isApplicable(_procedure_idx, projection);
}

} } // namespaces

