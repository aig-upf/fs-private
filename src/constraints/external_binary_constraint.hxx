
#pragma once

#include <constraints/external_constraint.hxx>
#include <actions.hxx>
#include <core_types.hxx>

namespace aptk { namespace core {

/**
 * A binary, externally-defined constraint.
 */
class ExternalBinaryConstraint : public ExternalConstraint
{
public:
	typedef std::shared_ptr<ExternalBinaryConstraint> cptr;
	
	ExternalBinaryConstraint(ApplicableEntity::cptr entity, unsigned procedure_idx);
	
	virtual ~ExternalBinaryConstraint() {}
	
	Constraint::Output filter(const DomainMap& domains);
	
	//! This actually arc-reduces the constraint with respect to the relevant variable given by the index
	Constraint::Output filter(unsigned variable = std::numeric_limits<unsigned>::max());
};

} } // namespaces

