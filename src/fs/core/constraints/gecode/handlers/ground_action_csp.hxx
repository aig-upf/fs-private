
#pragma once

#include <fs/core/constraints/gecode/handlers/base_action_csp.hxx>
#include <fs/core/actions/actions.hxx> // Necessary so that the return of get_action can be identified as covariant with that of the overriden method

namespace fs0 {
class GroundAction;
}

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class GroundActionCSP : public BaseActionCSP {
public:
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSP>> create(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_index, bool approximate, bool novelty);

	//! Constructors / Destructor
	GroundActionCSP(const GroundAction& action, const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions);
	~GroundActionCSP() = default;
	GroundActionCSP(const GroundActionCSP&) = delete;
	GroundActionCSP(GroundActionCSP&&) = delete;
	GroundActionCSP& operator=(const GroundActionCSP&) = delete;
	GroundActionCSP& operator=(GroundActionCSP&&) = delete;
	
	const GroundAction& get_action() const override { return _action; }
	
	const std::vector<const fs::ActionEffect*>& get_effects() const override;

	const fs::Formula* get_precondition() const override;
	
	//! Posts a X=x constraint into the given CSP
	GecodeCSP* post(VariableIdx variable, const object_id& value) const;
	
	//! Returns true iff the CSP has at least one solution
	static bool check_one_solution_exists(GecodeCSP* csp);

protected:
	
	const GroundAction& _action;
	
	std::vector<const fs::ActionEffect*> _add_effects;

	const ActionID* get_action_id(const GecodeCSP* solution) const override;
	
	//! Log some handler-related into
	virtual void log() const override;
};

} } // namespaces
