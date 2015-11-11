
#pragma once

#include <fs0_types.hxx>
#include <ostream>
#include <languages/fstrips/language.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace print {

//! Prints only the name + binding of an action, e.g. "move(b1, c2)"
class formula {
	protected:
		const fs::Formula& _formula;
	public:
		formula(const fs::Formula& formula) : _formula(formula) {}
		
		friend std::ostream& operator<<(std::ostream &os, const formula& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
