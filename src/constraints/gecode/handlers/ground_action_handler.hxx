
#pragma once

#include <constraints/gecode/handlers/base_action_handler.hxx>
#include <actions/actions.hxx> // Necessary so that the return of get_action can be identified as covariant with that of the overriden method

namespace fs0 {
class GroundAction;
}

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class GroundActionCSP : public BaseActionCSP {
public:
	typedef GroundActionCSP* ptr;
	
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSP>> create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty);

	//! Constructors / Destructor
	GroundActionCSP(const GroundAction& action, const TupleIndex& tuple_index, bool approximate, bool use_effect_conditions);
	virtual ~GroundActionCSP() {}
	
	const GroundAction& get_action() const override { return _action; }
	
	const std::vector<const fs::ActionEffect*>& get_effects() const override;

	const fs::Formula* get_precondition() const override;

protected:
	
	const GroundAction& _action;
	
	std::vector<const fs::ActionEffect*> _add_effects;

	const ActionID* get_action_id(const GecodeCSP* solution) const;
	
	//! Log some handler-related into
	virtual void log() const;
};

} } // namespaces
