
#include <constraints/gecode/action_manager.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <heuristics/relaxed_plan/gecode_rpg_layer.hxx>
#include <constraints/gecode/helper.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>


namespace fs0 { namespace gecode {

GecodeActionManager::ptr GecodeActionManager::create(const GroundAction& action) {
	auto csp_handler = new GecodeActionCSPHandler(action);
	return new GecodeActionManager(csp_handler);
}

void GecodeActionManager::process(unsigned int actionIdx, const fs0::RelaxedState& layer, const GecodeRPGLayer& gecode_layer, fs0::RPGData& rpg) {
	FDEBUG("main", "Processing action " << _handler->getAction());

	SimpleCSP* csp = _handler->instantiate_csp(gecode_layer);

	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints

	if (!locallyConsistent) {
		FDEBUG("main", "The action CSP is locally inconsistent: " << print::csp(_handler->getTranslator(), *csp));
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
