
#include <constraints/gecode/action_manager.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/helper.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>
#include <utils/printers/actions.hxx>


namespace fs0 { namespace gecode {


std::vector<std::shared_ptr<GecodeActionManager>> GecodeActionManager::createActionCSPs(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<GecodeActionManager>> managers;
	managers.reserve(actions.size());
	for (const auto action:actions) {
		std::vector<GecodeActionCSPHandler::ptr> handlers{new GecodeActionCSPHandler(*action)};
		auto manager = std::make_shared<GecodeActionManager>(std::move(handlers));
		FDEBUG("main", "Generated CSP for action " << *action << std::endl <<  *manager << std::endl);
		managers.push_back(manager);
	}
	return managers;
}


std::vector<std::shared_ptr<GecodeActionManager>> GecodeActionManager::createEffectCSPs(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<GecodeActionManager>> managers;
	for (const auto action:actions) {
		std::vector<GecodeActionCSPHandler::ptr> handlers;
		
		for (unsigned idx = 0; idx < action->getEffects().size(); ++idx) {
			auto handler = new GecodeEffectCSPHandler(*action, idx);
			handlers.push_back(handler);
			FDEBUG("main", "Generated CSP for the effect #" << idx << " of action " << print::action_name(*action) << std::endl <<  *handler << std::endl);
		}
		managers.push_back(std::make_shared<GecodeActionManager>(std::move(handlers)));
	}
	return managers;
}

void GecodeActionManager::process(unsigned int actionIdx, const GecodeRPGLayer& layer, fs0::RPGData& rpg) const {
	unsigned i = 0;
	for (auto handler:_handlers) {
		FFDEBUG("heuristic", "Processing effect: " << *(handler->getAction().getEffects().at(i)));
		process_handler(handler, actionIdx, layer, rpg);
		i++;
	}
}

void GecodeActionManager::process_handler(GecodeActionCSPHandler::ptr handler, unsigned int actionIdx, const GecodeRPGLayer& layer, fs0::RPGData& rpg) const {
	SimpleCSP* csp = handler->instantiate_csp(layer);

	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints

	if (!locallyConsistent) {
		FFDEBUG("heuristic", "The action CSP is locally inconsistent "); // << print::csp(handler->getTranslator(), *csp));
	} else {
		if (true) {  // Solve the CSP completely
			handler->compute_support(csp, actionIdx, rpg);
		} else { // Check only local consistency
			// TODO - Don't forget to delete the CSP in case of premature exit
			assert(0); // TODO Unimplemented
		}
	}
	delete csp;
}

std::ostream& GecodeActionManager::print(std::ostream& os) const {
	return _handlers[0]->print(os); // TODO
}



} } // namespaces
