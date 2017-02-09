
#pragma once

#include <fs_types.hxx>
#include <ostream>

namespace fs0 { class ActionData; class ActionBase; class GroundAction; }

namespace fs0 { namespace print {

//! Print only the header (e.g. "move(b: block, to: place)") of a lifted action
class action_data_name {
	protected:
		const ActionData& _action;

	public:
		action_data_name(const ActionData& action) : _action(action) {}
		
		friend std::ostream& operator<<(std::ostream &os, const action_data_name& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Print a full lifted action data, including preconditions and effects
class action_data {
	protected:
		const ActionData& _action;

	public:
		action_data(const ActionData& action) : _action(action) {}
		
		friend std::ostream& operator<<(std::ostream &os, const action_data& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Print the header of a possibly partially grounded action (e.g. "move(b, ?to)")
class action_header {
	protected:
		const ActionBase& _action;

	public:
		action_header(const ActionBase& action) : _action(action) {}
		
		friend std::ostream& operator<<(std::ostream &os, const action_header& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Print the header of a possibly partially grounded action (e.g. "move(b, ?to)")
class full_action {
	protected:
		const ActionBase& _action;

	public:
		full_action(const ActionBase& action) : _action(action) {}
		
		friend std::ostream& operator<<(std::ostream &os, const full_action& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

//! Print a list of ground actions
class actions {
	protected:
		const std::vector<const GroundAction*>& _actions;

	public:
		actions(const std::vector<const GroundAction*>& acts) : _actions(acts) {}
		
		friend std::ostream& operator<<(std::ostream &os, const actions& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

} } // namespaces
