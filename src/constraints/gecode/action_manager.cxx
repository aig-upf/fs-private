
#include <constraints/gecode/action_manager.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/helper.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>
#include <utils/printers/actions.hxx>
#include <utils/config.hxx>


namespace fs0 { namespace gecode {


std::vector<std::shared_ptr<GecodeManager>> GecodeActionManager::create(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<GecodeManager>> managers;
	managers.reserve(actions.size());
	
	for (unsigned idx = 0; idx < actions.size(); ++idx) {
		auto manager = std::make_shared<GecodeActionManager>(idx, Config::instance().useApproximateActionResolution(), new GecodeActionCSPHandler(*actions[idx]));
		FDEBUG("main", "Generated CSP for action " << *actions[idx] << std::endl <<  *manager << std::endl);
		managers.push_back(manager);
	}
	return managers;
}


std::vector<std::shared_ptr<GecodeManager>> GecodeActionEffectManager::create(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<GecodeManager>> managers;
	
	for (unsigned action_idx = 0; action_idx < actions.size(); ++action_idx) {
		const auto action = actions[action_idx];
		std::vector<GecodeActionCSPHandler::ptr> handlers;
		
		for (unsigned eff_idx = 0; eff_idx < action->getEffects().size(); ++eff_idx) {
			auto handler = new GecodeEffectCSPHandler(*action, eff_idx);
			handlers.push_back(handler);
			FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << print::action_name(*action) << std::endl <<  *handler << std::endl);
		}
		managers.push_back(std::make_shared<GecodeActionEffectManager>(action_idx, Config::instance().useApproximateActionResolution(), std::move(handlers)));
	}
	return managers;
}

void GecodeActionManager::process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const {
	FFDEBUG("heuristic", "Processing action #" << _action_idx << ": " << print::action_name(getAction()));
	process_handler(seed, _handler, layer, rpg);
}

void GecodeActionEffectManager::process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const {
	unsigned i = 0;
	FFDEBUG("heuristic", "Processing action #" << _action_idx << ": " << print::action_name(getAction()));
	for (auto handler:_handlers) {
		FFDEBUG("heuristic", "Processing effect: " << *(handler->getAction().getEffects().at(i)));
		process_handler(seed, handler, layer, rpg);
		++i;
	}
}

void GecodeManager::process_handler(const State& seed, GecodeActionCSPHandler::ptr handler, const GecodeRPGLayer& layer, RPGData& rpg) const {
	SimpleCSP* csp = handler->instantiate_csp(layer);

	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints

	if (!locallyConsistent) {
		FFDEBUG("heuristic", "The action CSP is locally inconsistent "); // << print::csp(handler->getTranslator(), *csp));
	} else {
		if (!_approximate) {  // Solve the CSP completely
			handler->compute_support(csp, _action_idx, rpg, seed);
		} else { // Check only local consistency
			handler->compute_approximate_support(csp, _action_idx, rpg, seed);
		}
	}
	delete csp;
}

std::ostream& GecodeActionManager::print(std::ostream& os) const {
	return _handler->print(os);
}

std::ostream& GecodeActionEffectManager::print(std::ostream& os) const {
	return _handlers[0]->print(os);
}



} } // namespaces
