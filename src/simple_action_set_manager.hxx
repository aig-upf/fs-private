
#pragma once

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <constraints/problem_constraints.hxx>
#include <utils/projections.hxx>


namespace aptk { namespace core {


/**
 * A simple manager that only checks applicability for non-relaxed states.
 * This means that we can get rid of causal data structures, etc.
 */
class SimpleActionSetManager
{
public:
	SimpleActionSetManager(const State& state, const ProblemConstraint::vctr& constraints)
		: _state(state), _constraints(constraints) {}
		
	SimpleActionSetManager(const SimpleActionSetManager& other)
		: _state(other._state), _constraints(other._constraints) {}
	
	//! Return true iff the preconditions of the applicable entity hold.
	bool checkPreconditionsHold(const ApplicableEntity& entity) const {
		for (unsigned idx = 0; idx < entity.getNumApplicabilityProcedures(); ++idx) {
			if (!isProcedureApplicable(entity, idx)) return false;
		}
		return true;
	}
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const CoreAction& action) const {
		if (!checkPreconditionsHold(action)) return false;
		
		if (_constraints.size() != 0) { // If we have no constraints, we can spare the cost of creating the new state.
			Changeset changeset;
			computeChangeset(action, changeset);
			State s1(_state, changeset);
			return checkStateConstraintsHold(s1);
		}
		return true;
	}
	
	bool checkStateConstraintsHold(const State& s) const {
		for (ProblemConstraint::cptr ctr:_constraints) {
			if (!ctr->isSatisfied(s)) return false;
		}
		return true;
	}
	
	void computeChangeset(const CoreAction& action, Changeset& changeset) const {
		for (unsigned idx = 0; idx < action.getNumEffectProcedures(); ++idx) {
			computeProcedureChangeset(idx, action, changeset);
		}
	}
	
	//!
	static void computeProcedurePointChangeset(unsigned procedureIdx, const CoreAction& action, const ProcedurePoint& point, Changeset& changeset) {
		const VariableIdxVector& affectedVars = action.getEffectAffectedVars(procedureIdx);
		ProcedurePoint affectedValues(affectedVars.size());
		// TODO - WHAT HAPPENS IF THE POINT WAS ACTUALLY NOT AFFECTED BY THE PROCEDURE??
		// TODO - WE'LL BE UNDERSTANDING THAT THE VALUE BECAME 0, WHILE ACTUALLY NOONE TOUCHED IT.
		// TODO - SOMETHING LIKE A MAP MIGHT HELP SOLVING IT, BUT IT'LL BE MORE EXPENSIVE.
		// TODO - OR A VECTOR OF PAIRS <AFFECTED_IDX, NEW_VALUE>
		action.applyEffectProcedure(procedureIdx, point, affectedValues);
		
		// Zip the new values for the affected variables into new facts and add them into the changeset.
		for (unsigned i = 0; i < affectedVars.size(); ++i) {
			changeset.add(Fact(affectedVars[i], affectedValues[i]));
		}
	}
	
protected:
	//! The state
	const State& _state;
	
	const ProblemConstraint::vctr& _constraints;

	//!
	bool isProcedureApplicable(const ApplicableEntity& entity, unsigned procedureIdx) const {
		const VariableIdxVector& relevant = entity.getApplicabilityRelevantVars(procedureIdx);
		assert(relevant.size() != 0); // Static applicability procedure that should have been detected in compilation time
		ObjectIdxVector point = Projections::project(_state, relevant);
		return entity.isApplicable(procedureIdx, point);
	}
	
	//!
	void computeProcedureChangeset(unsigned procedureIdx, const CoreAction& action, Changeset& changeset) const {
		const VariableIdxVector& relevant = action.getEffectRelevantVars(procedureIdx);
		ObjectIdxVector point = Projections::project(_state, relevant);
		computeProcedurePointChangeset(procedureIdx, action, point, changeset);
	}
};

} } // namespaces
