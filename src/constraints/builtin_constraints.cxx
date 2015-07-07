
#include <constraints/builtin_constraints.hxx>

namespace fs0 {

LTConstraint::LTConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryParametrizedScopedConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
ScopedConstraint::Output LTConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	
	Domain& x_dom = *(projection[0]);
	Domain& y_dom = *(projection[1]);
	ObjectIdx x_min = *(x_dom.cbegin()), x_max = *(x_dom.crbegin());
	ObjectIdx y_min = *(y_dom.cbegin()), y_max = *(y_dom.crbegin());
	
	if (variable == 0) { // We filter x_dom, the domain of X
		if (x_max < y_max) return Output::Unpruned;
		if (x_min >= y_max) return Output::Failure;
		
		// Otherwise there must be at least a value, but not all, in the new domain.
		auto it = x_dom.lower_bound(y_max); // it points to the first x in x_dom such that x >= y_max
		assert(it != x_dom.begin() && it != x_dom.end());
		x_dom = Domain(x_dom.begin(), it); // Update the domain by using the assignment operator.
		return Output::Pruned;
	} else { // We filter y_dom, the domain of Y
		if (x_min < y_min) return Output::Unpruned;
		if (x_min >= y_max) return Output::Failure;
		
		// Otherwise the domain has necessarily to be pruned, but is not inconsistent
		auto it = y_dom.lower_bound(x_min);
		assert(it != y_dom.end());
		if (*it == x_min) ++it;
		y_dom = Domain(it, y_dom.end());  // Update the domain by using the assignment operator.
		return Output::Pruned;
	}
}

LEQConstraint::LEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryParametrizedScopedConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
ScopedConstraint::Output LEQConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	
	Domain& x_dom = *(projection[0]);
	assert( x_dom.size() > 0 );
	Domain& y_dom = *(projection[1]);
	assert( y_dom.size() > 0 );
	ObjectIdx x_min = *(x_dom.cbegin()), x_max = *(x_dom.crbegin());
	ObjectIdx y_min = *(y_dom.cbegin()), y_max = *(y_dom.crbegin());
	
	if (variable == 0) { // We filter x_dom, the domain of X
		if (x_max <= y_max) return Output::Unpruned;
		if (x_min > y_max) return Output::Failure;
		
		// Otherwise there must be at least a value, but not all, in the new domain.
		auto it = x_dom.lower_bound(y_max);
		// assert(it != x_dom.begin() && it != x_dom.end()); // it points to the first x in x_dom such that x >= y_max
		// MRJ: The above is too strong, consider the case that the domain of y consists exactly of one value, and
		// it happens that it is the same as x_min (the first value in x_dom).
		assert( it != x_dom.end() );
		if (*it == y_max) ++it;
		x_dom = Domain(x_dom.begin(), it); // Update the domain by using the assignment operator.
		return Output::Pruned;
	} else { // We filter y_dom, the domain of Y
		if (x_min <= y_min) return Output::Unpruned;
		if (x_min > y_max) return Output::Failure;
		
		// Otherwise the domain has necessarily to be pruned, but is not inconsistent
		auto it = y_dom.lower_bound(x_min);
		assert(it != y_dom.begin() && it != y_dom.end());
		
		y_dom = Domain(it, y_dom.end());  // Update the domain by using the assignment operator.
		return Output::Pruned;
	}
}

EQConstraint::EQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryParametrizedScopedConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
ScopedConstraint::Output EQConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;
	
	for (ObjectIdx x:domain) {
		// We simply check that x is in the domain of the other variable
		auto it = other_domain.lower_bound(x);
		if (it != other_domain.end() && *it == x) { // `it` points to the first x in x_dom such that x >= y_max
			new_domain.insert(new_domain.cend(), x); //  x is an arc-consistent value. We will insert on the end of the container, as it is already sorted.
		}
	}
	if (new_domain.size() == domain.size()) return Output::Unpruned;
	if (new_domain.size() == 0) return Output::Failure;

	// Otherwise the domain has necessarily been pruned
	domain = new_domain; // Update the domain by using the assignment operator.
	return Output::Pruned;
}


NEQConstraint::NEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryParametrizedScopedConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
ScopedConstraint::Output NEQConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	
	if (other_domain.size() >= 2) return Output::Unpruned; // If the other domain has at least two domains, we won't be able to prune anything
	
	assert(other_domain.size() == 1);
	ObjectIdx other_val = *(other_domain.cbegin());
	
	// If we can erase the only value, i.e. it was in the domain, we do it, otherwise the result is an unpruned domain.
	if (domain.erase(other_val) == 0) return Output::Unpruned;
	return domain.size() > 0 ? Output::Pruned : Output::Failure;
}

EQXConstraint::EQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	UnaryParametrizedScopedConstraint(scope, parameters)
{
	assert(parameters.size() == 1);
}
	
// Filtering for a X = c constraint simply consists on pruning from the variable
// domain all values different than c.
ScopedConstraint::Output EQXConstraint::filter(const DomainMap& domains) const {
	assert(_scope.size() == 1);
	Domain& domain = *(domains.at(_scope[0]));
	auto it = domain.find(_binding[0]);
	if (it == domain.end()) {
		domain.clear(); // Just in case
		return Output::Failure;
	}
	
	if (domain.size() == 1) return Output::Unpruned; // 'c' is the only value in the set
	
	Domain new_domain;
	new_domain.insert(new_domain.cend(), _binding[0]);
	domain = new_domain; // Update the domain by using the assignment operator.	
	
	return Output::Pruned;
}

NEQXConstraint::NEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	UnaryParametrizedScopedConstraint(scope, parameters)
{
	assert(parameters.size() == 1);
}
	
// Filtering for a X <> c constraint simply consists on pruning from the variable
// domain value 'c', if available.
ScopedConstraint::Output NEQXConstraint::filter(const DomainMap& domains) const {
	assert(_scope.size() == 1);
	Domain& domain = *(domains.at(_scope[0]));
	unsigned erased = domain.erase(_binding[0]);
	if (erased == 0) return Output::Unpruned;
	else return (domain.size() == 0) ? Output::Failure : Output::Pruned;
}


} // namespaces
