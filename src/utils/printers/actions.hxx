
#pragma once

#include <fs0_types.hxx>
#include <ostream>

namespace fs0 { class GroundAction; class ActionSchema; }

namespace fs0 { namespace print {

//! Prints only the name + binding of an action, e.g. "move(b1, c2)"
class action_name {
	protected:
		const GroundAction& _action;

	public:
		action_name(const GroundAction& action) : _action(action) {}
		action_name(unsigned action_idx);
		
		friend std::ostream& operator<<(std::ostream &os, const action_name& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Prints only the name + binding of an action schema, e.g. "move(?b, ?to)"
class schema_name {
	protected:
		const ActionSchema& _schema;

	public:
		schema_name(const ActionSchema& action) : _schema(action) {}
		schema_name(unsigned schema_idx);
		
		friend std::ostream& operator<<(std::ostream &os, const schema_name& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};
} } // namespaces
