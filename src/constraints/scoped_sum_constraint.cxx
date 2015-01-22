

#include <constraints/scoped_sum_constraint.hxx>

namespace fs0 {


ScopedSumConstraint::ScopedSumConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	ScopedSumConstraint(scope) {}
	
ScopedSumConstraint::ScopedSumConstraint(const VariableIdxVector& scope)
	: ScopedConstraint(scope)
{
	if(scope.size() <= 1) throw std::runtime_error("The Sum global constraint requires at least two variables");
}

bool ScopedSumConstraint::isSatisfied(const ObjectIdxVector& values) const {
	assert(values.size() == _scope.size());
	int expected_sum_value = values.back();
	int total_sum  = std::accumulate(values.begin(), values.end(), 0);
	return expected_sum_value*2 == total_sum;
}


ScopedConstraint::Output ScopedSumConstraint::filter() {
	unsigned last_addend = _scope.size() - 1;
	
	Output output = Output::Unpruned;
	DomainPtr sum_domain = projection[last_addend]; // Get a pointer to the last domain, which is the result of the sum
	
	unsigned sum_mins = 0; // The sum of the min values of the domains
	
	for (unsigned i = 0, max = last_addend; i < max; ++i) {
		int minval = *(projection[i]->cbegin());
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
	for (unsigned i = 0, max = last_addend; i < max; ++i) {
		const auto dom = projection[i];
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

