
#include <constraints/gecode/handlers/action_base_handler.hxx>
#include <utils/logging.hxx>

namespace fs0 { namespace gecode {

BaseActionCSPHandler::BaseActionCSPHandler(const BaseAction& action, bool approximate)
	: BaseCSPHandler(approximate), _action(action)
{}

void BaseActionCSPHandler::process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const {
	log();
	
	SimpleCSP* csp = instantiate_csp(layer);

	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints

	if (!locallyConsistent) {
		FFDEBUG("heuristic", "The action CSP is locally inconsistent "); // << print::csp(handler->getTranslator(), *csp));
	} else {
		if (_approximate) {  // Check only local consistency
			//compute_approximate_support(csp, _action_idx, rpg, seed);
			// TODO - Unimplemented, but now sure it makes a lot of sense to solve the action CSPs approximately as of now
			throw UnimplementedFeatureException("Approximate support not yet implemented in action CSPs");
		} else { // Solve the CSP completely
			compute_support(csp, rpg, seed);
		}
	}
	delete csp;
}

} } // namespaces
