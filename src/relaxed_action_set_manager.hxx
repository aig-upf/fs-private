
#pragma once

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <constraints/constraints.hxx>
#include <utils/utils.hxx>

namespace aptk { namespace core {

	//!  A vector associating variable IDs to a possible subset of values from their domain.
	typedef std::map<unsigned, DomainSetVector> DomainSet;

/**
 * An action manager operating on a delete-free relaxation of the problem.
 */
class RelaxedActionSetManager
{
protected:
	//! The original state.
	const State* seed;

	//! The state constraints
	const ProblemConstraint::vctr& _constraints;

public:
	RelaxedActionSetManager(const ProblemConstraint::vctr& constraints)
		: seed(NULL), _constraints(constraints) {}
	
	RelaxedActionSetManager(const State* originalState, const ProblemConstraint::vctr& constraints)
		: seed(originalState), _constraints(constraints) {}

	RelaxedActionSetManager(const RelaxedActionSetManager& other)
		: seed(other.seed), _constraints(other._constraints)  {}
	
	
	//! Return true iff the preconditions of the applicable entity hold.
// 	bool checkPreconditionsHold(const ApplicableEntity& entity) const {
// 		for (unsigned idx = 0; idx < entity.getNumApplicabilityProcedures(); ++idx) {
// 			if (!isProcedureApplicable(entity, idx)) return false;
// 		}
// 	}
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
// 	bool isApplicable(const CoreAction& action) const;
	
	//!
	std::pair<bool, FactSetPtr> isApplicable(const ApplicableEntity& entity, DomainSet& domains) const;
	
	//!
	void computeChangeset(const CoreAction& action, const DomainSet& domains, Changeset& changeset) const;
	
	//!
	DomainSet projectValues(const RelaxedState& state, const ApplicableEntity& action) const;
	
protected:
	bool isProcedureApplicable(const ApplicableEntity& entity, DomainSet& domains, unsigned procedureIdx, FactSetPtr causes) const;
	
	LightDomainSet extractPoint(DomainSet& domains, const VariableIdxVector& variables) const;
	
	void computeProcedureChangeset(unsigned procedureIdx, const CoreAction& action, const DomainSet& domains, Changeset& changeset) const;
	void computeProcedurePointChangeset(unsigned procedureIdx, const CoreAction& action,
										const VariableIdxVector& relevant, const ProcedurePoint& point, Changeset& changeset) const;
};

} } // namespaces
