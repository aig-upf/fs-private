

#include <constraints/sum.hxx>

namespace fs0 {


SumConstraint::SumConstraint(unsigned arity) 
	: Constraint(arity)
{
	assert(_arity > 1);
}

bool SumConstraint::isSatisfied(const ObjectIdxVector&  values) const {
	assert(values.size() > 1);
	int sum_result = values.back();
	int total_sum  = std::accumulate(values.begin(), values.end(), 0) - sum_result;
	return sum_result == total_sum;
}


Constraint::Output SumConstraint::filter(const DomainVector& domains) {
	
	Output output = Output::Unpruned;
	DomainPtr sum_domain = domains[_arity-1]; // Get a pointer to the last domain, which is the result of the sum
	
	unsigned sum_mins = 0; // The sum of the min values of the domains
	
	for (unsigned i = 0, max = _arity-1; i < max; ++i) {
		int minval = *(domains[i]->cbegin());
		assert(minval >= 0);  // The current naive filtering algorithm only works for positive domains.
		sum_mins += (unsigned) minval;
	}
	
	assert(*(sum_domain->cbegin()) >= 0); // The min value of the sum domain.
	unsigned sum_maxval = (unsigned) *(sum_domain->crbegin());
	if (sum_maxval < sum_mins) return Output::Failure;


	// First filter the min values of the sum domain.
	Domain new_domain;
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
	for (unsigned i = 0, max = _arity-1; i < max; ++i) {
		const auto dom = domains[i];
		int max_allowed = sum_maxval - (sum_mins - *(dom->cbegin()));
		Domain new_domain;
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

} // namespaces

