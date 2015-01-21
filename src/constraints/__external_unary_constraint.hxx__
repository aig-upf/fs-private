
#pragma once

#include <constraints/external_constraint.hxx>
#include <actions.hxx>

namespace fs0 {

/**
 * A unary, externally-defined constraint.
 */
class ExternalUnaryConstraint : public ExternalConstraint
{
public:
	typedef std::shared_ptr<ExternalUnaryConstraint> cptr;
	
	ExternalUnaryConstraint(ApplicableEntity::cptr entity, unsigned procedure_idx);
	
	virtual ~ExternalUnaryConstraint() {}
	
	//! Filter (in-place) the given domains to remove inconsistent values.
	virtual Constraint::Output filter(const DomainMap& domains);
};

} // namespaces

