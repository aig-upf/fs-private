

#include <constraints/scoped_alldiff_constraint.hxx>

namespace fs0 {


ScopedAlldiffConstraint::ScopedAlldiffConstraint(const VariableIdxVector& scope) 
	: ScopedConstraint(scope), _arity(scope.size()), min(_arity), max(_arity), sorted_vars(_arity), u(_arity)
{}

bool ScopedAlldiffConstraint::isSatisfied(const ObjectIdxVector& values) const {
	assert(values.size() == _arity);
	std::set<ObjectIdx> distinct;
	for (ObjectIdx val:values) {
		auto res = distinct.insert(val);
		if (!res.second) return false; // We found a duplicate
	}
	return true;
}

// Computing bound consistent domains is done in two passes. The algorithm that computes new
// min is applied twice: first to the original problem, resulting into new min bounds, second to the problem
// where variables are replaced by their inverse, deducing max bounds.
ScopedConstraint::Output ScopedAlldiffConstraint::filter() {
	Output res = bounds_consistency(projection);
	if (res == Output::Failure) return res;
	
	invertDomains(projection);
	Output inverted_res = bounds_consistency(projection);
	if (inverted_res == Output::Failure) return inverted_res;
	else if (inverted_res == Output::Pruned) res = Output::Pruned;
	
	// Reinvert the domains again
	invertDomains(projection);
	
	return res;
}

//! Invert a domain, e.g. from D = {3, 4, 7} to D = {-7, -4, -3}
Domain ScopedAlldiffConstraint::invertDomain(const Domain& domain) const {
	Domain inverted;
	for (auto it = domain.crbegin(); it != domain.crend(); ++it) {
		inverted.insert(inverted.end(), -1 * (*it));
	}
	return inverted;
}

//! Invert all the variable domains.
void ScopedAlldiffConstraint::invertDomains(const DomainVector& domains) {
	for (unsigned i = 0; i < _arity; ++i) {
		*(domains[i]) = invertDomain(*(domains[i]));
	}
}


//! Sort the variables in increasing order of the max value of their domain, leaving them in the `sorted_vars` attribute.
void ScopedAlldiffConstraint::sortVariables(const DomainVector& domains) {
	std::iota(std::begin(sorted_vars), std::end(sorted_vars), 0); // fill the index vector with the range [0..num_vars-1]
	
	// A lambda function to sort based on the max domain value.
// 		const DomainVector& doms = domains; // To allow capture from the lambda expression
	auto sorter = [&domains](int x, int y) {
		const int max_x = *(domains[x]->crbegin()); // crbegin will get the max value, as the flat_set is sorted.
		const int max_y = *(domains[y]->crbegin());
		return max_x < max_y; 
	};
	std::sort(sorted_vars.begin(), sorted_vars.end(), sorter);
}

void ScopedAlldiffConstraint::updateBounds(const DomainVector& domains) {
	for (unsigned i = 0; i < _arity; ++i) {
		const unsigned var = sorted_vars[i];
		min[i] = *(domains[var]->cbegin());
		max[i] = *(domains[var]->crbegin());
	}
}


//! [a,b] is a Hall interval
ScopedConstraint::Output ScopedAlldiffConstraint::incrMin(const DomainVector& domains, int a, int b, unsigned i) {
	Output output = Output::Unpruned;
	for (unsigned j = i+1; j < _arity; ++j) {
		if (min[j] >= a) {
			// post x[j] >= b + 1
			const unsigned var = sorted_vars[j];
			Domain& old_domain = *(domains[var]);
			Domain new_domain;
			for (int val:old_domain) {
				if (val >= b+1) {
					new_domain.insert(new_domain.cend(), val);
				} else {
					output = Output::Pruned;
				}
			}
			old_domain = new_domain; // Copy with the assignment operator - more efficient than O(n) removals with cost O(n)
		}
	}
	return output;
}

ScopedConstraint::Output ScopedAlldiffConstraint::insert(const DomainVector& domains, unsigned i) {
	Output output = Output::Unpruned;
	u[i] = min[i];
	int bestMin = std::numeric_limits<int>::max();
	unsigned bestMinIdx = _arity + 1;
	for (unsigned j = 0; j < i; ++j) {
		if (min[j] < min[i]) {
			++u[j];
			if (u[j] > max[i]) return Output::Failure;
			if (u[j] == max[i] && min[j] < bestMin) {
				bestMin = min[j];
				bestMinIdx = j;
			}
		} else {
			++u[i];
		}
	}
	if (u[i] > max[i]) return Output::Failure;
	if (u[i] == max[i] && min[i] < bestMin) {
		bestMin = min[i];
		bestMinIdx = i;
	}
	
	if (bestMinIdx <= _arity) {
		output = incrMin(domains, bestMin, max[i], i);
	}
	return output;
}

ScopedConstraint::Output ScopedAlldiffConstraint::bounds_consistency(const DomainVector& domains) {
	Output output = Output::Unpruned;
	
	// 1. Sort the variables in increasing order of the max value of their domain.
	sortVariables(domains);
	
	// 2. Update the bounds (min and max values) for each variable
	updateBounds(domains);
	
	for (unsigned i = 0; i < _arity; ++i) {
		Output tmp = insert(domains, i);
		if (tmp == Output::Failure) return tmp;
		if (tmp == Output::Pruned) output = tmp;
	}
	
	return output;
}

} // namespaces

