
#include <constraints/compiled.hxx>
#include <utils/projections.hxx>

namespace fs0 {

CompiledUnaryConstraint::CompiledUnaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension) : 
	UnaryParametrizedScopedConstraint(scope, parameters), _extension(extension)
{}

CompiledUnaryConstraint::CompiledUnaryConstraint(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) :
	CompiledUnaryConstraint(constraint.getScope(), constraint.getParameters(), compile(constraint, problemInfo))
{}


CompiledUnaryConstraint::ExtensionT CompiledUnaryConstraint::compile(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) {
	VariableIdx relevant = constraint.getScope()[0];
	const ObjectIdxVector& all_values = problemInfo.getVariableObjects(relevant);
	
	std::set<ElementT> ordered;
	for(ObjectIdx value:all_values) {
		if (constraint.isSatisfied(value)) {
			ordered.insert(value);
		}
	}
	
	return ExtensionT(ordered.begin(), ordered.end());
}

bool CompiledUnaryConstraint::isSatisfied(ObjectIdx o) const {
	return std::binary_search(_extension.begin(), _extension.end(), o); // TODO - Change for a O(1) lookup in a std::unordered_set ?
}

ScopedConstraint::Output CompiledUnaryConstraint::filter(const DomainMap& domains) const {
	
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);
	Domain& domain = *(projection[0]);
	Domain new_domain;
	
	std::set_intersection(domain.begin(), domain.end(), _extension.begin(), _extension.end(), std::inserter(new_domain, new_domain.end()));
	
	if (new_domain.size() == domain.size()) return Output::Unpruned;
	if (new_domain.size() == 0) return Output::Failure;
	
	// Otherwise the domain has necessarily been pruned
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return Output::Pruned;
}

CompiledBinaryConstraint::CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension) :
	BinaryParametrizedScopedConstraint(scope, parameters), _extension(extension)
{}

CompiledBinaryConstraint::CompiledBinaryConstraint(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) :
	CompiledBinaryConstraint(constraint.getScope(), constraint.getParameters(), compile(constraint, problemInfo))
{}


bool CompiledBinaryConstraint::isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
	return std::binary_search(_extension.begin(), _extension.end(), std::make_pair(o1, o2)); // TODO - Change for a O(1) lookup in a std::unordered_set ?
}

CompiledBinaryConstraint::ExtensionT CompiledBinaryConstraint::compile(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) {
	
	VariableIdxVector scope = constraint.getScope();
	std::set<ElementT> ordered;
	
	for (ObjectIdx x:problemInfo.getVariableObjects(scope[0])) {
		for (ObjectIdx y:problemInfo.getVariableObjects(scope[1])) {
			if (constraint.isSatisfied(x, y)) {
				ordered.insert(std::make_pair(x, y));
			}
		}
	}
	
	return ExtensionT(ordered.begin(), ordered.end());
}

ScopedConstraint::Output CompiledBinaryConstraint::filter(unsigned variable) {
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
	
	if (new_domain.size() == domain.size()) return Output::Unpruned;
	if (new_domain.size() == 0) return Output::Failure;

	// Otherwise the domain has necessarily been pruned
	domain = new_domain; // Update the domain by using the assignment operator.
	return Output::Pruned;
}





} // namespaces
