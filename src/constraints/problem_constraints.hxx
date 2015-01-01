
#pragma once

#include <core_types.hxx>
#include <state.hxx>
#include <constraints/constraints.hxx>

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
	const VariableIdxVector _scope;
	
	//!
	DomainVector current_projection;
	
public:
	typedef std::shared_ptr<ProblemConstraint> cptr; // TODO : Put the const back
	typedef std::vector<ProblemConstraint::cptr> vctr;
	
	ProblemConstraint(Constraint* constraint, const VariableIdxVector& variables);
	
	virtual ~ProblemConstraint();
	
	inline const VariableIdxVector& getScope() const { return _scope; }
	
	inline unsigned getArity() const { return _scope.size(); }
	
	//! Returns true iff the current constraint is satisfied in the given state.
	virtual bool isSatisfied(const State& s) const;

	//! Filters from the set of currently loaded projections
	virtual Constraint::Output filter(unsigned variable = std::numeric_limits<unsigned>::max());
	
	//! Filters from a new set of domains.
	virtual Constraint::Output filter(const DomainMap& domains);
	
	//! Loads (i.e. caches a pointer of) the domain projections of the given state
	virtual void loadDomains(const DomainMap& domains);
};

} } // namespaces

