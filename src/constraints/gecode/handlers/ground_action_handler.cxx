
#include <constraints/gecode/handlers/ground_action_handler.hxx>
#include <utils/logging.hxx>
#include <actions/actions.hxx>
#include <actions/action_id.hxx>

namespace fs0 { namespace gecode {
	
std::vector<std::shared_ptr<BaseActionCSPHandler>> GroundActionCSPHandler::create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	
	for (unsigned idx = 0; idx < actions.size(); ++idx) {
		// auto x = new GroundActionCSPHandler(*actions[idx], approximate, novelty); std::cout << *x << std::endl;
		auto manager = std::make_shared<GroundActionCSPHandler>(*actions[idx], tuple_index, approximate);
		if (manager->init(novelty)) {
			FDEBUG("main", "Generated CSP for action " << *actions[idx] << std::endl <<  *manager << std::endl);
			managers.push_back(manager);
		} else {
			FDEBUG("main", "CSP for action " << *actions[idx] << " is inconsistent ==> the action is not applicable");
		}
	}
	return managers;
}

// If no set of effects is provided, we'll take all of them into account
GroundActionCSPHandler::GroundActionCSPHandler(const GroundAction& action, const TupleIndex& tuple_index, bool approximate)
	:  BaseActionCSPHandler(tuple_index, approximate), _action(action)
{}

const fs::Formula* GroundActionCSPHandler::get_precondition() const {
	return _action.getPrecondition();
}

const std::vector<const fs::ActionEffect*>& GroundActionCSPHandler::get_effects() const {
	return _action.getEffects();
}


const ActionID* GroundActionCSPHandler::get_action_id(const SimpleCSP* solution) const {
	return new PlainActionID(static_cast<const GroundAction*>(&_action)); // TODO Ugly
}

void GroundActionCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing action: " << _action);
}

} } // namespaces
