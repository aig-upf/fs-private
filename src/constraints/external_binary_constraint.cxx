

#include <constraints/external_binary_constraint.hxx>

namespace aptk { namespace core {

ExternalBinaryConstraint::ExternalBinaryConstraint(ApplicableEntity::cptr entity, unsigned procedure_idx) :
	ExternalConstraint(entity, procedure_idx)
{
	assert(_scope.size() == 2);
}

Constraint::Output ExternalBinaryConstraint::filter(const DomainMap& domains) {
	throw std::runtime_error("Binary constraints only support the other version of the filter method");
}

//! This actually arc-reduces the constraint with respect to the relevant variable given by the index
Constraint::Output ExternalBinaryConstraint::filter(unsigned variable) {
	assert(current_projection.size() == 2);
	assert(variable == 0 || variable == 1);
	
	unsigned other = (variable == 0) ? 1 : 0;
	
	Domain& domain = *(current_projection[variable]);
	Domain& other_domain = *(current_projection[other]);		
	Domain new_domain;
	
	Constraint::Output output = Constraint::Output::Unpruned;
	
	// A small helper to create the point having the values in the right order.
	auto make_point = [&variable](ObjectIdx x, ObjectIdx z) -> ProcedurePoint {
		if (variable == 0) {
			return {x, z};
		} else {
			return {z, x};
		}
	};
	
	for (ObjectIdx x:domain) {
		for (ObjectIdx z:other_domain) {
			if (_entity->isApplicable(_procedure_idx, make_point(x, z))) {
				new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
				break; // x is an arc-consistent value, so we can break the inner loop and continue to check the next possible value.
			}
		}
	}
	
	if (new_domain.size() == 0) return Constraint::Output::Failure;
	
	if (new_domain.size() != domain.size()) {
		domain = new_domain; // Update the domain by using the assignment operator.
		output = Constraint::Output::Pruned;
	}
	
	return output;
}

} } // namespaces

