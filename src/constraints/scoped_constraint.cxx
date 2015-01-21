
#include <constraints/scoped_constraint.hxx>
#include <utils/projections.hxx>

namespace fs0 {

ScopedConstraint::ScopedConstraint(const VariableIdxVector& scope) :
	_scope(scope), projection() {}
	
ExternalScopedConstraint::ExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ScopedConstraint(scope), _binding(parameters) {}
		
		
UnaryExternalScopedConstraint::UnaryExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ExternalScopedConstraint(scope, parameters) {}

// bool UnaryExternalScopedConstraint::isSatisfied(const ObjectIdxVector& values) const {
// 	assert(values.size()==1);
// 	return this->isSatisfied(values[0]);
// }

//! Filters from a new set of domains.
Constraint::Output UnaryExternalScopedConstraint::filter(const DomainMap& domains) const {
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);
	
	Domain new_domain;
	Domain& domain = *(projection[0]);
	
	Constraint::Output output = Constraint::Output::Unpruned;
	
	for (ObjectIdx value:domain) {
		if (this->isSatisfied({value})) {
			new_domain.insert(new_domain.cend(), value); // We will insert on the end of the container, as it is already sorted.
		} else {
			output = Constraint::Output::Pruned; // Mark the result as "pruned", but keep iterating to prune more values
		}
	}
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return (domain.size() == 0) ? Constraint::Output::Failure : output;		
};

BinaryExternalScopedConstraint::BinaryExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ExternalScopedConstraint(scope, parameters) {}


//! This actually arc-reduces the constraint with respect to the relevant variable given by the index
Constraint::Output BinaryExternalScopedConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	
	unsigned other = (variable == 0) ? 1 : 0;
	
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);		
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
			if (this->isSatisfied(make_point(x, z))) {
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



} // namespaces

