
#include <constraints/builtin.hxx>

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
	Domain& y_dom = *(projection[1]);
	ObjectIdx x_min = *(x_dom.cbegin()), x_max = *(x_dom.crbegin());
	ObjectIdx y_min = *(y_dom.cbegin()), y_max = *(y_dom.crbegin());
	
	if (variable == 0) { // We filter x_dom, the domain of X
		if (x_max <= y_max) return Output::Unpruned;
		if (x_min > y_max) return Output::Failure;
		
		// Otherwise there must be at least a value, but not all, in the new domain.
		auto it = x_dom.lower_bound(y_max);
		assert(it != x_dom.begin() && it != x_dom.end()); // it points to the first x in x_dom such that x >= y_max
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

} // namespaces
