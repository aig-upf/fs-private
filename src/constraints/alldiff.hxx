
#pragma once

#include <memory>
#include <vector>
#include <constraints/constraints.hxx>

namespace aptk { namespace core {


/**
 * An alldifferent custom propagator. Currently supports only Puget's bound consistency algorithm 2 from
 * 
 * Puget, J. F. (1998, July). A fast algorithm for the bound consistency of alldiff constraints. In AAAI/IAAI (pp. 359-366).
 * 
 * with a complexity of O(n^2), where n is the number of variables.
 */
class AlldiffConstraint : public Constraint
{
protected:
	//! Vectors to store min and max domain values.
	std::vector<int> min, max;
	
	//! The variables sorted by increasing max domain value
	std::vector<VariableIdx> sorted_vars;
	
	//! The variables sorted by increasing max domain value
	std::vector<int> u;
	
	//! the variable domains themselves
	LightDomainSet doms;
	
	
public:
	AlldiffConstraint(const VariableIdxVector& variables) 
		: Constraint(variables), min(num_variables), max(num_variables), sorted_vars(num_variables), u(num_variables)
	{}
	
	// Computing bound consistent domains is done in two passes. The algorithm that computes new
	// min is applied twice: first to the originM problem, resulting into new min bounds, second to the problem
	// where variables are replaced by their inverse, deducing max bounds.
	Output enforce_consistency(DomainSet& domains) {
		deIndexDomains(domains);
		
		Output res = bounds_consistency();
		
		if (res == Output::Failure) return res;
		
		
		invertDomains();
		Output inverted_res = bounds_consistency();
		if (inverted_res == Output::Failure) return inverted_res;
		else if (inverted_res == Output::Pruned) res = Output::Pruned;
		
		// Reinvert the domains again
		invertDomains();
		
		return res;
	}
	
	
	//! Mainly for testing / debugging purposes. Highly non-optimized!
	std::vector<ObjectIdxVector> getCurrentDomains() {
		std::vector<ObjectIdxVector> domains;
		for (auto d:doms) {
			domains.push_back(ObjectIdxVector(d->cbegin(), d->cend()));
		}
		return domains;
	}
	
	
protected:
	//! For performance reasons, we move from a map-based representation of variable domains to a vector-based one.
	void deIndexDomains(DomainSet& domains) {
		doms.clear();
		doms.reserve(domains.size());
		
		for (unsigned i = 0; i < _variables.size(); ++i) {
			VariableIdx index = _variables[i];
			doms.push_back(&(domains[index]));
		}
	}
	
	//! Invert a domain, e.g. from D = {3, 4, 7} to D = {-7, -4, -3}
	DomainSetVector invertDomain(const DomainSetVector& domain) {
		DomainSetVector inverted;
		for (auto it = domain.crbegin(); it != domain.crend(); ++it) {
			inverted.insert(inverted.end(), -1 * (*it));
		}
		return inverted;
	}
	
	//! Invert all the variable domains.
	void invertDomains() {
		for (unsigned i = 0; i < num_variables; ++i) {
			*(doms[i]) = invertDomain(*(doms[i]));
		}
	}


	//! Sort the variables in increasing order of the max value of their domain, leaving them in the `sorted_vars` attribute.
	void sortVariables() {
		std::iota(std::begin(sorted_vars), std::end(sorted_vars), 0); // fill the index vector with the range [0..num_vars-1]
		
		// A lambda function to sort based on the max domain value.
		const auto& domains = doms; // To allow capture from the lambda expression
		auto sorter = [&domains](int x, int y) {
			const int max_x = *(domains[x]->crbegin()); // crbegin will get the max value, as the flat_set is sorted.
			const int max_y = *(domains[y]->crbegin());
			return max_x < max_y; 
		};
		std::sort(sorted_vars.begin(), sorted_vars.end(), sorter);
	}
	
	void updateBounds() {
		for (unsigned i = 0; i < num_variables; ++i) {
			const unsigned var = sorted_vars[i];
			min[i] = *(doms[var]->cbegin());
			max[i] = *(doms[var]->crbegin());
		}
	}
	
	
	//! [a,b] is a Hall interval
	Output incrMin(int a, int b, unsigned i) {
		Output output = Output::Unpruned;
		for (unsigned j = i+1; j < num_variables; ++j) {
			if (min[j] >= a) {
				// post x[j] >= b + 1
				const unsigned var = sorted_vars[j];
				DomainSetVector new_domain;
				for (int val:(*doms[var])) {
					if (val >= b+1) {
						new_domain.insert(new_domain.cend(), val);
					} else {
						output = Output::Pruned;
					}
				}
				*doms[var] = new_domain; // Copy with the assignment operator - more efficient than O(n) removals with cost O(n)
			}
		}
		return output;
	}
	
	Output insert(unsigned i) {
		Output output = Output::Unpruned;
		u[i] = min[i];
		for (unsigned j = 0; j < i; ++j) {
			if (min[j] < min[i]) {
				++u[j];
				if (u[j] > max[i]) return Output::Failure;
				if (u[j] == max[i]) {
					if (incrMin(min[j], max[i], i) == Output::Pruned) {
						output = Output::Pruned;
					}
				}
			} else {
				++u[i];
			}
		}
		if (u[i] > max[i]) return Output::Failure;
		if (u[i] == max[i]) {
			if (incrMin(min[i], max[i], i) == Output::Pruned) {
				output = Output::Pruned;
			}
		}
		return output;
	}

	Output insert2(unsigned i) {
		Output output = Output::Unpruned;
		u[i] = min[i];
		int bestMin = std::numeric_limits<int>::max();
		unsigned bestMinIdx = num_variables + 1;
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
		
		if (bestMinIdx <= num_variables) {
			output = incrMin(bestMin, max[i], i);
		}
		return output;
	}
	
	Output bounds_consistency() {
		Output output = Output::Unpruned;
		
		// 1. Sort the variables in increasing order of the max value of their domain.
		sortVariables();
		
		// 2. Update the bounds (min and max values) for each variable
		updateBounds();
		
		for (unsigned i = 0; i < num_variables; ++i) {
			Output tmp = insert2(i);
			if (tmp == Output::Failure) return tmp;
			if (tmp == Output::Pruned) output = tmp;
		}
 		
		return output;
	}
};


} } // namespaces

