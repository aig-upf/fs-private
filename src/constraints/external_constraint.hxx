
#pragma once

#include <constraints/problem_constraints.hxx>
#include <actions.hxx>

namespace fs0 {

/**
 * A constraint defined through an external procedure, i.e. a semantic attachment.
 * In a sense, currently this is just a wrapper on top of an applicability entity, each entity having as many external
 * constraints as applicability procedures.
 */
class ExternalConstraint : public ProblemConstraint
{
protected:
	//! The "external" entity that implicitly defines the constraint.
	ApplicableEntity::cptr _entity;
	
	//! The index of the procedure in the external entity that corresponds to the current constraint.
	unsigned _procedure_idx;
	
public:
	typedef std::shared_ptr<ExternalConstraint> cptr;
	
	//! Note that the arity of the constraint is given by the number of applicability procedures of the external entity.
	ExternalConstraint(ApplicableEntity::cptr entity, unsigned procedure_idx);
	
	virtual ~ExternalConstraint() {}
	
	//! Returns true iff the current constraint is satisfied in the given state.
	bool isSatisfied(const State& s) const;
};

} // namespaces

