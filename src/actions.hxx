/*
Lightweight Automated Planning Toolkit
Copyright (C) 2012
Miquel Ramirez <miquel.ramirez@rmit.edu.au>
Nir Lipovetzky <nirlipo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <iosfwd>
#include <limits>
#include <memory>
#include <vector>

#include <core_types.hxx>
#include <state.hxx>
#include <fact.hxx>

namespace aptk { namespace core {

class Changeset;

class ApplicableEntity
{
public:
	typedef std::shared_ptr<const ApplicableEntity> cptr;
	
	ApplicableEntity(const std::vector<VariableIdxVector>& appRelevantVars);
	
	ApplicableEntity(const std::vector<VariableIdxVector>& appRelevantVars, VariableIdxVector&& allRelevantVars);
	
	//! Keep it virtual!
	virtual ~ApplicableEntity() {};
	
	virtual unsigned getNumApplicabilityProcedures() const { return _appRelevantVars.size(); }
	
	//! Tells whether the action's given applicability procedure is satisfied under the values contained in `relevant`.
	virtual bool isApplicable(unsigned procedureIdx, const ProcedurePoint& relevant) const = 0;
	
	virtual const VariableIdxVector& getApplicabilityRelevantVars(unsigned procedureIdx) const { return _appRelevantVars[procedureIdx]; };
	
	const VariableIdxVector& getAllRelevantVariables() const { return _allRelevantVars; }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ApplicableEntity&  entity) { return entity.print(os); }
	virtual std::ostream& print(std::ostream& os) const;

protected:
	//! One VariableIdxVector per each applicability procedure, containing the indexes of those state variables 
	//! relevant to that procedure.
	const std::vector<VariableIdxVector> _appRelevantVars;
	
	//! The indexes of _all_ the state variables relevant to at least one of the effect or applicability procedures of the action.
	const VariableIdxVector _allRelevantVars;
};

/**
 * 
 */
class CoreAction : public ApplicableEntity
{
public:
	static const ActionIdx INVALID_ACTION;
	
	typedef std::shared_ptr<const CoreAction> cptr;
	
	CoreAction(const ObjectIdxVector& binding,
			   const ObjectIdxVector& derived,
			   const std::vector<VariableIdxVector>& appRelevantVars,
			   const std::vector<VariableIdxVector>& effRelevantVars,
			   const std::vector<VariableIdxVector>& effAffectedVars
	);
	
	//! Keep it virtual!
	virtual ~CoreAction() {};
	
	//! Returns the name of the action. To be implemented in each concrete action.
	virtual const std::string& getName() const = 0;
	
	//! Returns the signature of the action. To be implemented in each concrete action.
	//! An action signature is a vector v of types, where v[i] denotes the type of the action's i-th parameter.
	virtual const ActionSignature& getSignature() const = 0;
	
	//! Apply an effect procedure.
	virtual void applyEffectProcedure(unsigned procedureIdx, const ProcedurePoint& relevant, ProcedurePoint& affected) const = 0;
	
	
	virtual unsigned getNumEffectProcedures() const { return _effRelevantVars.size(); }
	
	virtual const VariableIdxVector& getEffectRelevantVars(unsigned procedureIdx) const { return _effRelevantVars[procedureIdx]; };
	
	virtual const VariableIdxVector& getEffectAffectedVars(unsigned procedureIdx) const { return _effAffectedVars[procedureIdx]; };

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const CoreAction&  action) { return action.print(os); }
	virtual std::ostream& print(std::ostream& os) const;
	

protected:
	//! The indexes of the action binding, if any.
	const ObjectIdxVector _binding;
	
	//! The indexes of the objects derived from the binding, if any.
	const ObjectIdxVector _derived;
	
	//! One VariableIdxVector per each effect procedure, containing the indexes of those state variables relevant to that procedure.
	const std::vector<VariableIdxVector> _effRelevantVars;
	
	//! The indexes of the state variables affected by the action effects
	const std::vector<VariableIdxVector> _effAffectedVars;
};


typedef std::vector<CoreAction::cptr> ActionList;

} } // namespaces
