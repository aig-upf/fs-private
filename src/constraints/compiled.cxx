
#include <constraints/compiled.hxx>
#include <utils/projections.hxx>
#include <utils/utils.hxx>

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

CompiledBinaryConstraint::CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension1, ExtensionT&& extension2) :
	BinaryParametrizedScopedConstraint(scope, parameters), _extension1(extension1),  _extension2(extension2)
{}

CompiledBinaryConstraint::CompiledBinaryConstraint(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) :
	CompiledBinaryConstraint(constraint.getScope(), constraint.getParameters(), compile(constraint, 0, problemInfo), compile(constraint, 1, problemInfo))
{}


bool CompiledBinaryConstraint::isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
	auto iter = _extension1.find(o1);
	assert(iter != _extension1.end());
	const ObjectIdxVector& D_y = iter->second; // iter->second contains all the elements y of the domain of the second variable such that <x, y> satisfies the constraint
	return std::binary_search(D_y.begin(), D_y.end(), o2); // TODO - Change for a O(1) lookup in a std::unordered_set ?
}

CompiledBinaryConstraint::ExtensionT CompiledBinaryConstraint::compile(const BinaryParametrizedScopedConstraint& constraint, unsigned variable, const ProblemInfo& problemInfo) {
	
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	VariableIdxVector scope = constraint.getScope();
	
	std::map<ObjectIdx, std::set<ObjectIdx>> ordered;
	
	for (ObjectIdx x:problemInfo.getVariableObjects(scope[variable])) {
		auto res = ordered.insert(std::make_pair(x, std::set<ObjectIdx>())); // We insert the empty vector (all elements will at least have it) and keep the reference.
		assert(res.second); // The element couldn't exist
		std::set<ObjectIdx>& set = res.first->second;
		
		for (ObjectIdx y:problemInfo.getVariableObjects(scope[other])) {
			if (constraint.isSatisfied(x, y)) {
				set.insert(y);
			}
		}
	}
	
	// Now we transform the ordered set into a (implicitly ordered) vector
	ExtensionT extension;
	for(const auto& elem:ordered) {
		extension.insert(std::make_pair(elem.first,
			ObjectIdxVector(elem.second.begin(), elem.second.end())
		));
	}
	return extension;
}

/*
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
*/


ScopedConstraint::Output CompiledBinaryConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	const ExtensionT& extension_map = (variable == 0) ? _extension1 : _extension2;
	
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;
	
	for (ObjectIdx x:domain) {
		auto iter = extension_map.find(x);
		assert(iter != extension_map.end());
		const ObjectIdxVector& D_y = iter->second; // iter->second contains all the elements y of the domain of the second variable such that <x, y> satisfies the constraint
		if (!Utils::empty_intersection(other_domain.begin(), other_domain.end(), D_y.begin() ,D_y.end())) {
			new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
			break; // x is an arc-consistent value, so we can break the inner loop and continue to check the next possible value.			
		}
	}
	
	if (new_domain.size() == domain.size()) return Output::Unpruned;
	if (new_domain.size() == 0) return Output::Failure;

	// Otherwise the domain has necessarily been pruned
	domain = new_domain; // Update the domain by using the assignment operator.
	return Output::Pruned;
}





} // namespaces
