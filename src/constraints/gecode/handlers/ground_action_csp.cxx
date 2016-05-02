
#include <languages/fstrips/effects.hxx>
#include <constraints/gecode/handlers/ground_action_csp.hxx>
#include <aptk2/tools/logging.hxx>
#include <actions/actions.hxx>
#include <actions/action_id.hxx>


namespace fs0 { namespace gecode {
	
std::vector<std::shared_ptr<BaseActionCSP>> GroundActionCSP::create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<BaseActionCSP>> managers;
	
	for (unsigned idx = 0; idx < actions.size(); ++idx) {
		// auto x = new GroundActionCSP(*actions[idx], approximate, novelty, false); std::cout << *x << std::endl;
		// When creating an action CSP handler, it doesn't really make much sense to use the effect conditions.
		auto manager = std::make_shared<GroundActionCSP>(*actions[idx], tuple_index, approximate, false);
		if (manager->init(novelty)) {
			LPT_DEBUG("main", "Generated CSP for action " << *actions[idx] << std::endl <<  *manager << std::endl);
			managers.push_back(manager);
		} else {
			LPT_DEBUG("main", "CSP for action " << *actions[idx] << " is inconsistent ==> the action is not applicable");
		}
	}
	return managers;
}

// If no set of effects is provided, we'll take all of them into account
GroundActionCSP::GroundActionCSP(const GroundAction& action, const TupleIndex& tuple_index, bool approximate, bool use_effect_conditions)
	:  BaseActionCSP(tuple_index, approximate, use_effect_conditions), _action(action)
{
	// Filter out delete effects
	for (const fs::ActionEffect* effect:_action.getEffects()) {
		if (!effect->is_del())
		_add_effects.push_back(effect);
	}
}

const fs::Formula* GroundActionCSP::get_precondition() const {
	return _action.getPrecondition();
}

const std::vector<const fs::ActionEffect*>& GroundActionCSP::get_effects() const {
	return _add_effects;
}


const ActionID* GroundActionCSP::get_action_id(const GecodeCSP* solution) const {
	return new PlainActionID(&_action);
}

void GroundActionCSP::log() const {
	LPT_EDEBUG("heuristic", "Processing action: " << _action);
}

} } // namespaces
