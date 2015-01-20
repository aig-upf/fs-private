
#include <constraints/scoped_constraint.hxx>
#include <utils/projections.hxx>

namespace fs0 {

ScopedConstraint::ScopedConstraint(const VariableIdxVector& scope) :
	_scope(scope), projection() {}
	
ExternalScopedConstraint::ExternalScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ScopedConstraint(scope), _parameters(parameters) {}
		
		
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

} // namespaces

