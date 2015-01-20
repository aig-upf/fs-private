
#pragma once

#include <iosfwd>
#include <limits>
#include <memory>
#include <vector>

#include <fs0_types.hxx>
#include <state.hxx>
#include <fact.hxx>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {

class Changeset;

class AppEntity
{
public:
	typedef std::shared_ptr<const AppEntity> cptr;
	
	AppEntity(const std::vector<ScopedConstraint const *>& constraints);
	
	//! Keep it virtual!
	virtual ~AppEntity();
	
	virtual unsigned getNumConstraints() const { return _constraints.size(); }
	
	//! Tells whether the action's given applicability procedure is satisfied under the values contained in `relevant`.
// 	virtual bool isApplicable(unsigned procedureIdx, const ProcedurePoint& relevant) const = 0;
	
// 	virtual const VariableIdxVector& getApplicabilityRelevantVars(unsigned procedureIdx) const { return _appRelevantVars[procedureIdx]; };
	
	const VariableIdxVector& getAllRelevantVariables() const { return _allRelevantVars; }
	
	//! Prints a representation of the object to the given stream.
// 	friend std::ostream& operator<<(std::ostream &os, const AppEntity&  entity) { return entity.print(os); }
// 	virtual std::ostream& print(std::ostream& os) const;

protected:
	//! One VariableIdxVector per each applicability procedure, containing the indexes of those state variables 
	//! relevant to that procedure.
	const std::vector<ScopedConstraint const *> _constraints;
	
	//! The indexes of _all_ the state variables relevant to at least one of the effect or applicability procedures of the action.
	const VariableIdxVector _allRelevantVars;
	
	VariableIdxVector extractRelevantVariables(const std::vector<ScopedConstraint const *>& constraints);
};


} // namespaces
