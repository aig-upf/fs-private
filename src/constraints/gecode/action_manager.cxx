
#include <constraints/gecode/action_manager.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/helper.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>


namespace fs0 { namespace gecode {

std::shared_ptr<GecodeActionManager> GecodeActionManager::create(const GroundAction& action) {
	return std::make_shared<GecodeActionManager>(new GecodeActionCSPHandler(action));
}

std::vector<std::shared_ptr<GecodeActionManager>> GecodeActionManager::create(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<GecodeActionManager>> managers;
	managers.reserve(actions.size());
	for (const auto action:actions) {
		auto manager = create(*action);
		FDEBUG("main", "Generated CSP for action " << *action << std::endl <<  *manager << std::endl);
		managers.push_back(manager);
	}
	return managers;
}


void GecodeActionManager::process(unsigned int actionIdx, const GecodeRPGLayer& layer, fs0::RPGData& rpg) const {
	FFDEBUG("main", "Processing action " << _handler->getAction());

	SimpleCSP* csp = _handler->instantiate_csp(layer);

	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints

	if (!locallyConsistent) {
		FFDEBUG("heuristic", "The action CSP is locally inconsistent: " << print::csp(_handler->getTranslator(), *csp));
	} else {
		if (true) {  // Solve the CSP completely
			_handler->compute_support(csp, actionIdx, rpg);
		} else { // Check only local consistency
			// TODO - Don't forget to delete the CSP in case of premature exit
			assert(0); // TODO Unimplemented
		}
	}
	delete csp;
}

const GroundAction& GecodeActionManager::getAction() const {
	return _handler->getAction();
}

std::ostream& GecodeActionManager::print(std::ostream& os) const {
	return _handler->print(os);
}



} } // namespaces
