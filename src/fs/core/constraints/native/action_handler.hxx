
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/languages/fstrips/language_fwd.hxx>
#include <fs/core/languages/fstrips/effects.hxx>

namespace fs0 {
class GroundAction;
class AtomIndex;
}

namespace fs0 { namespace gecode {


//!
class ActionHandler {
public:
	//! Factory method
	static std::vector<std::shared_ptr<ActionHandler>> create(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_index);

	//! Constructors / Destructor
	ActionHandler(const GroundAction& action, const AtomIndex& tuple_index);
	~ActionHandler() = default;
	ActionHandler(const ActionHandler&) = delete;
	ActionHandler(ActionHandler&&) = delete;
	ActionHandler& operator=(const ActionHandler&) = delete;
	ActionHandler& operator=(ActionHandler&&) = delete;
	
	const GroundAction& get_action() const { return _action; }
	
	const std::vector<const fs::ActionEffect*>& get_effects() const;

	const fs::Formula* get_precondition() const;

protected:
	
	const GroundAction& _action;
	
	std::vector<const fs::ActionEffect*> _add_effects;

    std::vector<std::pair<VariableIdx, object_id>> _equality_atoms;
    std::vector<std::pair<VariableIdx, object_id>> _inequality_atoms;


	//! Log some handler-related into
	virtual void log() const;
};

} } // namespaces
