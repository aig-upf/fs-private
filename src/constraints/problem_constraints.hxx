
#pragma once

#include <core_types.hxx>
#include <state.hxx>
#include <utils/utils.hxx>
#include "constraints.hxx"

namespace aptk { namespace core {

/**
 * 
 */
class ProblemConstraint
{
protected:
	// The actual constraint
	Constraint* ctr;
	
	//! The state variables that make up the scope of the constraint.
	const VariableIdxVector scope;
	
public:
	typedef std::shared_ptr<ProblemConstraint> cptr; // TODO : Put the const back
	typedef std::vector<ProblemConstraint::cptr> vctr;
	
	ProblemConstraint(Constraint* constraint, const VariableIdxVector& variables) 
		: ctr(constraint), scope(variables) {}
	
	~ProblemConstraint() {
		delete ctr;
	}
	
// 	const VariableIdxVector& getScope() { return scope; }
	const VariableIdxVector& getScope() { return ctr->getScope(); }
	
	//! Returns true iff the current constraint is satisfied in the given state.
	bool isSatisfied(const State& s) const {
		return ctr->isSatisfied(Utils::extractVariables(s, scope));
	}
	
	// TODO - Refactor
	virtual Constraint::Output enforce_consistency(DomainSet& domains) {
		return ctr->enforce_consistency(domains);
	}
};

} } // namespaces

