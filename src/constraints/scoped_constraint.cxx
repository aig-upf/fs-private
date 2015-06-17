
#include <constraints/scoped_constraint.hxx>
#include <constraints/compiled.hxx>
#include <utils/projections.hxx>
#include <constraints/scoped_effect.hxx>
#include <problem_info.hxx>

namespace fs0 {

ScopedConstraint::ScopedConstraint(const VariableIdxVector& scope) :
	_scope(scope), projection() {}

ParametrizedScopedConstraint::ParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ScopedConstraint(scope), _binding(parameters) {}


UnaryParametrizedScopedConstraint::UnaryParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ParametrizedScopedConstraint(scope, parameters) {}

bool UnaryParametrizedScopedConstraint::isSatisfied(const ObjectIdxVector& values) const {
	assert(values.size()==1);
	return this->isSatisfied(values[0]);
}

//! Filters from a new set of domains.
ScopedConstraint::Output UnaryParametrizedScopedConstraint::filter(const DomainMap& domains) const {
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);

	Domain new_domain;
	Domain& domain = *(projection[0]);

	Output output = Output::Unpruned;

	for (ObjectIdx value:domain) {
		if (this->isSatisfied(value)) {
			new_domain.insert(new_domain.cend(), value); // We will insert on the end of the container, as it is already sorted.
		} else {
			output = Output::Pruned; // Mark the result as "pruned", but keep iterating to prune more values
		}
	}
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return (domain.size() == 0) ? Output::Failure : output;
}

BinaryParametrizedScopedConstraint::BinaryParametrizedScopedConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ParametrizedScopedConstraint(scope, parameters) {}


bool BinaryParametrizedScopedConstraint::isSatisfied(const ObjectIdxVector& values) const {
	assert(values.size()==2);
	return this->isSatisfied(values[0], values[1]);
}

//! This actually arc-reduces the constraint with respect to the relevant variable given by the index
ScopedConstraint::Output BinaryParametrizedScopedConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);

	unsigned other = (variable == 0) ? 1 : 0;

	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;

	for (ObjectIdx x:domain) {
		for (ObjectIdx z:other_domain) {
			// We need to invoke isSatisfied with the parameters in the right order
			if ((variable == 0 && this->isSatisfied(x, z)) || (variable == 1 && this->isSatisfied(z, x))) {
				new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
				break; // x is an arc-consistent value, so we can break the inner loop and continue to check the next possible value.
			}
		}
	}

	if (new_domain.size() == 0) return Output::Failure;
	if (new_domain.size() != domain.size()) {
		domain = new_domain; // Update the domain by using the assignment operator.
		return Output::Pruned;
	}
	return Output::Unpruned;
}

UnaryDomainBoundsConstraint::UnaryDomainBoundsConstraint(const UnaryScopedEffect* effect, const ProblemInfo& problemInfo) :
	UnaryParametrizedScopedConstraint(effect->getScope(), {}), _problemInfo(problemInfo), _effect(effect)
{}

// The domain constraint is satisfied iff the application of the corresponding effect keeps the affected
// value within the domain bounds.
bool UnaryDomainBoundsConstraint::isSatisfied(ObjectIdx o) const {
		return _effect->applicable(o) && _problemInfo.checkValueIsValid(_effect->apply(o));
}

VariableIdx
UnaryDomainBoundsConstraint::getAffected() const {
	return _effect->getAffected();
}

BinaryDomainBoundsConstraint::BinaryDomainBoundsConstraint(const BinaryScopedEffect* effect, const ProblemInfo& problemInfo) :
	BinaryParametrizedScopedConstraint(effect->getScope(), {}), _problemInfo(problemInfo), _effect(effect)
{}

bool BinaryDomainBoundsConstraint::isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
		return _effect->applicable(o1, o2) &&  _problemInfo.checkValueIsValid(_effect->apply(o1, o2));
}

VariableIdx
BinaryDomainBoundsConstraint::getAffected() const {
	return _effect->getAffected();
}

ScopedConstraint::cptr UnaryParametrizedScopedConstraint::compile(const ProblemInfo& problemInfo) const {
	return nullptr;
	return new CompiledUnaryConstraint(*this, problemInfo);
}

ScopedConstraint::cptr BinaryParametrizedScopedConstraint::compile(const ProblemInfo& problemInfo) const {
	return nullptr;
	return new CompiledBinaryConstraint(*this, problemInfo);
}

} // namespaces
