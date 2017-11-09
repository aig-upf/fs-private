
#include <fs/core/constraints//direct/constraint.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/utils//projections.hxx>
#include <fs/core/state.hxx>
#include <fs/core/constraints//direct/compiled.hxx>

namespace fs0 {

DirectConstraint::DirectConstraint(const VariableIdxVector& scope)
	: DirectConstraint(scope, {}) {}
	
DirectConstraint::DirectConstraint(const VariableIdxVector& scope, const ValueTuple& parameters)
	: DirectComponent(scope, parameters) {}


void DirectConstraint::loadDomains(const DomainMap& domains) const {
	projection = Projections::project(domains, _scope);
}

//! Filters from a new set of domains.
FilteringOutput UnaryDirectConstraint::filter(const DomainMap& domains) const {
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);

	Domain new_domain;
	Domain& domain = *(projection[0]);

	FilteringOutput output = FilteringOutput::Unpruned;

	for (const object_id&
 value:domain) {
		if (this->isSatisfied(value)) {
			new_domain.insert(new_domain.cend(), value); // We will insert on the end of the container, as it is already sorted.
		} else {
			output = FilteringOutput::Pruned; // Mark the result as "pruned", but keep iterating to prune more values
		}
	}
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return (domain.size() == 0) ? FilteringOutput::Failure : output;
}


//! This actually arc-reduces the constraint with respect to the relevant variable given by the index
FilteringOutput BinaryDirectConstraint::filter(unsigned variable) const {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);

	unsigned other = (variable == 0) ? 1 : 0;

	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;

	for (const object_id&
 x:domain) {
		for (const object_id&
 z:other_domain) {
			// We need to invoke isSatisfied with the parameters in the right order
			if ((variable == 0 && this->isSatisfied(x, z)) || (variable == 1 && this->isSatisfied(z, x))) {
				new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
				break; // x is an arc-consistent value, so we can break the inner loop and continue to check the next possible value.
			}
		}
	}

	if (new_domain.size() == 0) return FilteringOutput::Failure;
	if (new_domain.size() != domain.size()) {
		domain = new_domain; // Update the domain by using the assignment operator.
		return FilteringOutput::Pruned;
	}
	return FilteringOutput::Unpruned;
}

UnaryDirectConstraint::UnaryDirectConstraint(const VariableIdxVector& scope, const ValueTuple& parameters) :
	DirectConstraint(scope, parameters) {
	assert(scope.size() == 1);
}

DirectConstraint* UnaryDirectConstraint::compile(const ProblemInfo& problemInfo) const {
	return nullptr;
}

BinaryDirectConstraint::BinaryDirectConstraint(const VariableIdxVector& scope, const ValueTuple& parameters) :
	DirectConstraint(scope, parameters) {
	assert(scope.size() == 2);
}
	
DirectConstraint* BinaryDirectConstraint::compile(const ProblemInfo& problemInfo) const {
	return nullptr;
}


} // namespaces
