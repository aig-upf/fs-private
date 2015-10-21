

#pragma once

#include <fs0_types.hxx>
#include <problem.hxx>

namespace fs0 { namespace gecode {

//! A compact class that indexes the number of occurrences of state variables in our CSPs,
//! making a distinction between direct and derived state variables, and encapsulating the logic
//! that allows us to decide when a certain (derived) state variable is nullable or not.
class VariableCounter {
public:
	VariableCounter(bool use_dont_care) : _use_dont_care(use_dont_care) {}
	
	
	void count_direct(VariableIdx variable) { count(variable, _as_direct); }
	void count_derived(VariableIdx variable) { count(variable, _as_derived); }
	
	unsigned num_direct_occurrences(VariableIdx variable) const { return num_occurrences(variable, _as_direct); }
	unsigned num_derived_occurrences(VariableIdx variable) const { return num_occurrences(variable, _as_derived); }
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const VariableCounter& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const {
		auto printer = [&os](const std::pair<VariableIdx, unsigned> count) {
			os <<  "\t\t" << Problem::getInfo().getVariableName(count.first) << ": " << count.second << std::endl;
		};
		
		os << "\tDirect State Variables: " << std::endl;
		std::for_each(_as_direct.cbegin(), _as_direct.cend(), printer);
		
		os << "\tDerived State Variables: " << std::endl;
		std::for_each(_as_derived.cbegin(), _as_derived.cend(), printer);
		
		return os;
	}
	
	bool useDontCare() const { return _use_dont_care; }
	
	//! Returns true iff the given variable is nullable (i.e. with a dont-care value) for the given CSP
	bool is_nullable(VariableIdx variable) const {
		if (!_use_dont_care) return false;
		unsigned direct = num_direct_occurrences(variable);
		unsigned derived = num_derived_occurrences(variable);
		// We should not have counted more than one direct occurrence of the same state variable, given that we count only unique terms.
		assert(direct < 2);
		
		// The variable can only be assigned a DON'T CARE value if it appears one single time as a derived variable.
		return derived == 1 && direct == 0;
	}

protected:
	std::map<VariableIdx, unsigned> _as_direct;
	std::map<VariableIdx, unsigned> _as_derived;
	
	static void count(VariableIdx variable, std::map<VariableIdx, unsigned>& where) {
		auto it = where.find(variable);
		if (it == where.end()) where.insert(std::make_pair(variable, 1));
		else it->second++;
	}
	
	static unsigned num_occurrences(VariableIdx variable, const std::map<VariableIdx, unsigned>& where) {
		auto it = where.find(variable);
		if (it == where.end()) return 0;
		return it->second;
	}
	
	bool _use_dont_care;
	
};


} } // namespaces


