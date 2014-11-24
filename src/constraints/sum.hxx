
#pragma once

#include <memory>
#include <vector>
#include <constraints/constraints.hxx>

namespace aptk { namespace core {


/**
 * A Sum constraint custom propagator. 
 */
class SumConstraint : public Constraint
{
protected:
	//! the variable domains themselves
	ULightDomainSet doms;
	
	//! 
	DomainSetVector* sum_domain;
	
public:
	SumConstraint(const VariableIdxVector& variables) 
		: Constraint(variables)
	{}
	
	Output enforce_consistency(DomainSet& domains) {
		Output output = Output::Unpruned;
		
		assert(num_variables > 0);
		deIndexDomains(domains);
		
		unsigned sum_mins = 0; // The sum of the min values of the domains
		for (const auto dom:doms) {
			int minval = *(dom->cbegin());
			assert(minval >= 0);  // The current filtering algorithm only works for positive domains.
			sum_mins += (unsigned) minval;
		}
		
		assert(*(sum_domain->cbegin()) >= 0); // The min value of the sum domain.
		unsigned sum_maxval = (unsigned) *(sum_domain->crbegin());
		if (sum_maxval < sum_mins) return Output::Failure;


		// First filter the min values of the sum domain.
		DomainSetVector new_domain;
		for (int val:(*sum_domain)) {
			if ((unsigned) val >= sum_mins) {
				new_domain.insert(new_domain.cend(), val);
			} else {
				output = Output::Pruned;
			}
		}
		if (new_domain.size() == 0) return Output::Failure;
		*sum_domain = new_domain;
		
		// Now filter the max values of the regular domains
		for (const auto dom:doms) {
			int max_allowed = sum_maxval - (sum_mins - *(dom->cbegin()));
			DomainSetVector new_domain;
			for (int val:(*dom)) {
				if (val <= max_allowed) {
					new_domain.insert(new_domain.cend(), val);
				} else {
					output = Output::Pruned;
				}
				if (new_domain.size() == 0) return Output::Failure;
				*dom = new_domain;
			}
		}
		return output;
	}
	
	
	//! Mainly for testing / debugging purposes. Highly non-optimized!
	std::vector<std::vector<int>> getCurrentDomains() {
		std::vector<std::vector<int>> domains;
		for (auto d:doms) {
			domains.push_back(std::vector<int>(d->cbegin(), d->cend()));
		}
		domains.push_back(std::vector<int>(sum_domain->cbegin(), sum_domain->cend()));
		return domains;
	}
	
	
protected:
	//! For performance reasons, we move from a map-based representation of variable domains to a vector-based one.
	//! We distinguish between the domain corresponding to the sum variable and the rest of them.
	void deIndexDomains(DomainSet& domains) {
		doms.clear();
		doms.reserve(domains.size());
		
		for (unsigned i = 0; i < _variables.size() - 1; ++i) {
			VariableIdx index = _variables[i];
			doms.push_back(&(domains[index]));
		}
		
		// The last variable is the result of the sum
		VariableIdx sum_idx = _variables[_variables.size()-1];
		sum_domain = &(domains[sum_idx]);
	}

};


} } // namespaces

