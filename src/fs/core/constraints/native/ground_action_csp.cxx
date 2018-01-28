
#include <fs/core/constraints/native/action_handler.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/actions/actions.hxx>


namespace fs0 { namespace gecode {
	
std::vector<std::shared_ptr<ActionHandler>> ActionHandler::create(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_index) {
	std::vector<std::shared_ptr<ActionHandler>> managers;
	
	for (unsigned idx = 0; idx < actions.size(); ++idx) {
		// When creating an action CSP handler, it doesn't really make much sense to use the effect conditions.
		auto manager = std::make_shared<ActionHandler>(*actions[idx], tuple_index);
//		if (manager->init(novelty)) {
//			LPT_DEBUG("main", "Generated CSP for action " << *actions[idx] << std::endl <<  *manager << std::endl);
//			managers.push_back(manager);
//		} else {
//			LPT_DEBUG("main", "CSP for action " << *actions[idx] << " is inconsistent ==> the action is not applicable");
//		}
	}
	return managers;
}

// If no set of effects is provided, we'll take all of them into account
ActionHandler::ActionHandler(const GroundAction& action, const AtomIndex& tuple_index)
	: _action(action)
{
	// Filter out delete effects
	for (const fs::ActionEffect* effect:_action.getEffects()) {
		if (!effect->is_del()) {
            _add_effects.push_back(effect);
        }
	}
}

const fs::Formula* ActionHandler::get_precondition() const {
	return _action.getPrecondition();
}

const std::vector<const fs::ActionEffect*>& ActionHandler::get_effects() const {
	return _add_effects;
}

void ActionHandler::log() const {
	LPT_EDEBUG("heuristic", "Processing action: " << _action);
}


} } // namespaces
