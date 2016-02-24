
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/handlers/ground_action_handler.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/scopes.hxx>
#include <actions/ground_action.hxx>
#include <actions/action_id.hxx>
#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <gecode/driver.hh>

namespace fs0 { namespace gecode {
	
std::vector<std::shared_ptr<BaseActionCSPHandler>> GroundActionCSPHandler::create(const std::vector<GroundAction::cptr>& actions, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
	
	for (unsigned idx = 0; idx < actions.size(); ++idx) {
		// auto x = new GroundActionCSPHandler(*actions[idx], approximate, novelty); std::cout << *x << std::endl;
		auto manager = std::make_shared<GroundActionCSPHandler>(*actions[idx], approximate, novelty);
		FDEBUG("main", "Generated CSP for action " << *actions[idx] << std::endl <<  *manager << std::endl);
		managers.push_back(manager);
	}
	return managers;
}

GroundActionCSPHandler::GroundActionCSPHandler(const GroundAction& action, const std::vector<fs::ActionEffect::cptr>& effects, bool approximate, bool novelty)
	:  BaseActionCSPHandler(action, effects, approximate, novelty)
{}

// If no set of effects is provided, we'll take all of them into account
GroundActionCSPHandler::GroundActionCSPHandler(const GroundAction& action,  bool approximate, bool novelty)
	:  GroundActionCSPHandler(action, action.getEffects(), approximate, novelty)
{}

const ActionID* GroundActionCSPHandler::get_action_id(SimpleCSP* solution) const {
	return new PlainActionID(_action.getId());
}

void GroundActionCSPHandler::log() const {
	FFDEBUG("heuristic", "Processing action #" << _action.getId() << ": " << _action.fullname());
}

} } // namespaces
