
#include <algorithm>
#include <problem.hxx>
#include <constraints/direct/sum_constraint.hxx>
#include <utils/printers/helper.hxx>

namespace fs0 {


SumConstraint::SumConstraint(const VariableIdxVector& scope, const std::vector<ObjectIdx>& parameters) :
	SumConstraint(scope) {}

SumConstraint::SumConstraint(const VariableIdxVector& scope)
	: DirectConstraint(scope)
{
	if(scope.size() <= 1) throw std::runtime_error("The Sum global constraint requires at least two variables");
}


FilteringOutput SumConstraint::filter() {
	unsigned last_addend = _scope.size() - 1;

	FilteringOutput output = FilteringOutput::Unpruned;
	DomainPtr sum_domain = projection[last_addend]; // Get a pointer to the last domain, which is the result of the sum

	unsigned sum_mins = 0; // The sum of the min values of the domains

	for (unsigned i = 0, max = last_addend; i < max; ++i) {
		int minval = boost::get<int>(*(projection[i]->cbegin()));
		assert(minval >= 0);  // The current naive filtering algorithm only works for positive domains.
		sum_mins += (unsigned) minval;
	}

	assert(boost::get<int>(*(sum_domain->cbegin())) >= 0); // The min value of the sum domain.
	unsigned sum_maxval = (unsigned) boost::get<int>(*(sum_domain->crbegin()));
	if (sum_maxval < sum_mins) return FilteringOutput::Failure;


	// First filter the min values of the sum domain.
	Domain new_domain;
	for (ObjectIdx wrapped_val:(*sum_domain)) {
        int val = boost::get<int>(wrapped_val);
		if ((unsigned) val >= sum_mins) {
			new_domain.insert(new_domain.cend(), val);
		} else {
			output = FilteringOutput::Pruned;
		}
	}
	if (new_domain.size() == 0) return FilteringOutput::Failure;
	*sum_domain = new_domain;

	// Now filter the max values of the regular domains
	for (unsigned i = 0, max = last_addend; i < max; ++i) {
		const auto dom = projection[i];
		int max_allowed = sum_maxval - (sum_mins - boost::get<int>(*(dom->cbegin())));
		Domain new_domain;
		for (ObjectIdx wrapped_val:(*dom)) {
            int val = boost::get<int>(wrapped_val);
			if (val <= max_allowed) {
				new_domain.insert(new_domain.cend(), val);
			} else {
				output = FilteringOutput::Pruned;
			}
			if (new_domain.size() == 0) return FilteringOutput::Failure;
			*dom = new_domain;
		}
	}
	return output;
}

std::ostream& SumConstraint::print(std::ostream& os) const {
	const auto& names = print::Helper::name_variables(_scope);
	for (unsigned i = 0; i < names.size() - 1; ++i) {
		const auto& varname = names[i];
		os << varname;
		if (i < names.size() - 2) os << " + ";
	}
	os << " = " << names[names.size()-1];
	return os;
}

} // namespaces
