

#include <constraints/external_unary_constraint.hxx>
#include <utils/projections.hxx>

namespace aptk { namespace core {

ExternalUnaryConstraint::ExternalUnaryConstraint(ApplicableEntity::cptr entity, unsigned procedure_idx) :
	ExternalConstraint(entity, procedure_idx)
{
	assert(_scope.size() == 1);
}

//! Filter (in-place) the given domains to remove inconsistent values.
Constraint::Output ExternalUnaryConstraint::filter(const DomainMap& domains) {
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);
	
	Domain new_domain;
	Domain& domain = *(projection[0]);
	
	Constraint::Output output = Constraint::Output::Unpruned;
	
	for (ObjectIdx obj:domain) {
		if (_entity->isApplicable(_procedure_idx, {obj})) {
			new_domain.insert(new_domain.cend(), obj); // We will insert on the end of the container, as it is already sorted.
		} else {
			output = Constraint::Output::Pruned;
		}
	}
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return (domain.size() == 0) ? Constraint::Output::Failure : output;
}

} } // namespaces

