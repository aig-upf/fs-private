
#pragma once

#include <ostream>

namespace fs0 { namespace language { namespace fstrips { class Formula; class Axiom; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace print {

class axiom_header {
	protected:
		const fs::Axiom& _axiom;

	public:
		axiom_header(const fs::Axiom& axiom) : _axiom(axiom) {}
		
		friend std::ostream& operator<<(std::ostream &os, const axiom_header& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

} } // namespaces
