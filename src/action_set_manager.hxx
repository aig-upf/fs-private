
#ifndef __APTK_CORE_ACTION_SET_MANAGER_HXX__
#define __APTK_CORE_ACTION_SET_MANAGER_HXX__

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_changeset.hxx>
#include <core_types.hxx>
#include <fact.hxx>
#include <variable_cache.hxx>
#include <justified_action.hxx>

namespace aptk { namespace core {


/**
 * A simple manager that only checks applicability for non-relaxed states.
 * This means that we can get rid of causal data structures, etc.
 */
class SimpleActionSetManager
{
public:
	SimpleActionSetManager(const State& state): 
		_state(state) {}
		
	SimpleActionSetManager(const SimpleActionSetManager& other): 
		_state(other._state) {}
	
	bool isApplicable(const ApplicableEntity& entity) const {
		for (unsigned idx = 0; idx < entity.getNumApplicabilityProcedures(); ++idx) {
			if (!isProcedureApplicable(entity, idx)) return false;
		}
		return true;
	}
	bool isApplicable(const JustifiedApplicableEntity& entity) const { return isApplicable(entity.getEntity()); }
	
	void computeChangeset(const CoreAction& action, Changeset& changeset) const {
		assert(isApplicable(action));
		
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

	//!
	bool isProcedureApplicable(const ApplicableEntity& entity, unsigned procedureIdx) const {
		const VariableIdxVector& relevant = entity.getApplicabilityRelevantVars(procedureIdx);
		assert(relevant.size() != 0); // Static applicability procedure that should have been detected in compilation time
		ProcedurePoint point = extractPoint(relevant);
		return entity.isApplicable(procedureIdx, point);
	}
	
	//!
	void computeProcedureChangeset(unsigned procedureIdx, const CoreAction& action, Changeset& changeset) const {
		const VariableIdxVector& relevant = action.getEffectRelevantVars(procedureIdx);
		ProcedurePoint point = extractPoint(relevant);
		computeProcedurePointChangeset(procedureIdx, action, point, changeset);
	}
	
	//!
	ProcedurePoint extractPoint(const VariableIdxVector& variables) const {
		ProcedurePoint values;
		for (auto& idx:variables) {
			values.push_back(_state.getValue(idx));
		}
		return values;
	}
};


/**
 */
class RelaxedActionSetManager
{
protected:
	//! The original state.
	const State* _originalState;

	//! The state
	const RelaxedState& _state;

public:
	RelaxedActionSetManager(const RelaxedState& state) : _originalState(NULL), _state(state) {}
	RelaxedActionSetManager(const State* originalState, const RelaxedState& state) : _originalState(originalState), _state(state) {}
	
	RelaxedActionSetManager(const RelaxedActionSetManager& other) : _originalState(other._originalState), _state(other._state) {}
	
	//!
	bool isApplicable(JustifiedApplicableEntity& justified) const;
	
	//!
	void computeChangeset(const JustifiedAction& justified, Changeset& changeset) const;
	
protected:
	bool isProcedureApplicable(JustifiedApplicableEntity& justified, unsigned procedureIdx) const;
	bool isMonadicProcedureApplicable(JustifiedApplicableEntity& justified, unsigned procedureIdx) const;
	
	std::vector<VariableIdxVector> extractPoint(const JustifiedApplicableEntity& justified, const VariableIdxVector& variables) const;
	std::vector<ObjectIdx>& extractMonadicPoint(JustifiedApplicableEntity& justified, VariableIdx variable) const;
	
	void computeProcedureChangeset(unsigned procedureIdx, const JustifiedAction& justified, Changeset& changeset) const;
	void computeProcedurePointChangeset(unsigned procedureIdx, const JustifiedAction& justified, 
										const VariableIdxVector& relevant, const ProcedurePoint& point, Changeset& changeset) const;
	void computeMonadicProcedureChangeset(unsigned procedureIdx, const JustifiedAction& justified, Changeset& changeset) const;
};

} } // namespaces

#endif
