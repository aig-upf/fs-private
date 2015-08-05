
#pragma once

#include <fs0_types.hxx>
#include <ostream>

namespace fs0 { class GroundAction; }

namespace fs0 { namespace print {

//! Prints only the name + binding of an action, e.g. "move(b1, c2)"
class action_name {
	protected:
		const GroundAction& _action;
	public:
		action_name(const GroundAction& action) : _action(action) {}
		
		friend std::ostream& operator<<(std::ostream &os, const action_name& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
