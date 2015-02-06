
#include <constraints/builtin.hxx>

namespace fs0 {


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
		
		// Otherwise there must be at least a value, but not all in the new domain.
		auto it = x_dom.lower_bound(y_max);
		assert(it != x_dom.begin() && it != x_dom.end());
		if (*it == y_max) ++it;
		x_dom = Domain(x_dom.begin(), it); // Update the domain by using the assignment operator.
		return Output::Pruned;
	} else { // We filter y_dom, the domain of X
		if (x_min <= y_min) return Output::Unpruned;
		if (x_min > y_max) return Output::Failure;
		
		// Otherwise the domain has necessarily to be pruned, but is not inconsistent
		auto it = y_dom.lower_bound(x_min);
		assert(it != y_dom.begin() && it != y_dom.end());
		y_dom = Domain(it, y_dom.end());  // Update the domain by using the assignment operator.
		return Output::Pruned;
	}
// 	} else  {
// 		if (x_max <= y_max) return Output::Unpruned;
// 		
// 		Domain new_domain;
// 		for (ObjectIdx x:x_dom) {
// 			if (x > y_max) break; // Because the set is ordered no further x will fulfill the condition.
// 			new_domain.insert(new_domain.cend(), x); // We will insert on the end of the container, as it is already sorted.
// 		}
// 		
// 		if (new_domain.size() == 0) return Output::Failure;
// 
// 		// Otherwise the domain has necessarily been pruned
// 		x_dom = new_domain; // Update the domain by using the assignment operator.
// 		return Output::Pruned;
}

} // namespaces
