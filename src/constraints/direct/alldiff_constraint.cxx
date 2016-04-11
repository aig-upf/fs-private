

#include <constraints/direct/alldiff_constraint.hxx>
#include <utils/printers/helper.hxx>
#include <utils/printers/vector.hxx>
#include <algorithm>

namespace fs0 {


AlldiffConstraint::AlldiffConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) 
	: AlldiffConstraint(scope) {}

AlldiffConstraint::AlldiffConstraint(const VariableIdxVector& scope) 
	: DirectConstraint(scope), _arity(scope.size()), min(_arity), max(_arity), _sorted_vars(_arity), u(_arity)
{}

// Computing bound consistent domains is done in two passes. The algorithm that computes new
// min is applied twice: first to the original problem, resulting into new min bounds, second to the problem
// where variables are replaced by their inverse, deducing max bounds.
FilteringOutput AlldiffConstraint::filter() {
	FilteringOutput res = bounds_consistency(projection);
	if (res == FilteringOutput::Failure) return res;
	
	invertDomains(projection);
	FilteringOutput inverted_res = bounds_consistency(projection);
	if (inverted_res == FilteringOutput::Failure) return inverted_res;
	else if (inverted_res == FilteringOutput::Pruned) res = FilteringOutput::Pruned;
	
	// Reinvert the domains again
	invertDomains(projection);
	
	return res;
}

//! Invert a domain, e.g. from D = {3, 4, 7} to D = {-7, -4, -3}
Domain AlldiffConstraint::invertDomain(const Domain& domain) const {
	Domain inverted;
	for (auto it = domain.crbegin(); it != domain.crend(); ++it) {
		inverted.insert(inverted.end(), -1 * (*it));
	}
	return inverted;
}

//! Invert all the variable domains.
void AlldiffConstraint::invertDomains(const DomainVector& domains) {
	for (unsigned i = 0; i < _arity; ++i) {
		*(domains[i]) = invertDomain(*(domains[i]));
	}
}


//! Sort the variables in increasing order of the max value of their domain, leaving them in the `_sorted_vars` attribute.
void AlldiffConstraint::sortVariables(const DomainVector& domains) {
	std::iota(std::begin(_sorted_vars), std::end(_sorted_vars), 0); // fill the index vector with the range [0..num_vars-1]
	
	// A lambda function to sort based on the max domain value.
// 		const DomainVector& doms = domains; // To allow capture from the lambda expression
	auto sorter = [&domains](int x, int y) {
		const int max_x = *(domains[x]->crbegin()); // crbegin will get the max value, as the flat_set is sorted.
		const int max_y = *(domains[y]->crbegin());
		return max_x < max_y; 
	};
	std::sort(_sorted_vars.begin(), _sorted_vars.end(), sorter);
}

void AlldiffConstraint::updateBounds(const DomainVector& domains) {
	for (unsigned i = 0; i < _arity; ++i) {
		const unsigned var = _sorted_vars[i];
		min[i] = *(domains[var]->cbegin());
		max[i] = *(domains[var]->crbegin());
	}
}


//! [a,b] is a Hall interval
FilteringOutput AlldiffConstraint::incrMin(const DomainVector& domains, int a, int b, unsigned i) {
	FilteringOutput output = FilteringOutput::Unpruned;
	for (unsigned j = i+1; j < _arity; ++j) {
		if (min[j] >= a) {
			// post x[j] >= b + 1
			const unsigned var = _sorted_vars[j];
			Domain& old_domain = *(domains[var]);
			Domain new_domain;
			for (int val:old_domain) {
				if (val >= b+1) {
					new_domain.insert(new_domain.cend(), val);
				} else {
					output = FilteringOutput::Pruned;
				}
			}
			old_domain = new_domain; // Copy with the assignment operator - more efficient than O(n) removals with cost O(n)
		}
	}
	return output;
}

FilteringOutput AlldiffConstraint::insert(const DomainVector& domains, unsigned i) {
	FilteringOutput output = FilteringOutput::Unpruned;
	u[i] = min[i];
	int bestMin = std::numeric_limits<int>::max();
	unsigned bestMinIdx = _arity + 1;
	for (unsigned j = 0; j < i; ++j) {
		if (min[j] < min[i]) {
			++u[j];
			if (u[j] > max[i]) return FilteringOutput::Failure;
			if (u[j] == max[i] && min[j] < bestMin) {
				bestMin = min[j];
				bestMinIdx = j;
			}
		} else {
			++u[i];
		}
	}
	if (u[i] > max[i]) return FilteringOutput::Failure;
	if (u[i] == max[i] && min[i] < bestMin) {
		bestMin = min[i];
		bestMinIdx = i;
	}
	
	if (bestMinIdx <= _arity) {
		output = incrMin(domains, bestMin, max[i], i);
	}
	return output;
}

FilteringOutput AlldiffConstraint::bounds_consistency(const DomainVector& domains) {
	FilteringOutput output = FilteringOutput::Unpruned;
	
	// 1. Sort the variables in increasing order of the max value of their domain.
	sortVariables(domains);
	
	// 2. Update the bounds (min and max values) for each variable
	updateBounds(domains);
	
	for (unsigned i = 0; i < _arity; ++i) {
		FilteringOutput tmp = insert(domains, i);
		if (tmp == FilteringOutput::Failure) return tmp;
		if (tmp == FilteringOutput::Pruned) output = tmp;
	}
	
	return output;
}

std::ostream& AlldiffConstraint::print(std::ostream& os) const {
	os << "alldiff(" << print::container(print::Helper::name_variables(_scope)) << ")";
	return os;
}

} // namespaces

