
#include <constraints/gecode/action_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <constraints/gecode/helper.hxx>
#include <utils/logging.hxx>


namespace fs0 { namespace gecode {

GecodeActionManager::ptr GecodeActionManager::create(const GroundAction& action) {
	auto csp_handler = new GecodeActionCSPHandler(action);
	return new GecodeActionManager(csp_handler);
}

void GecodeActionManager::process(unsigned actionIdx, const RelaxedState& layer, RPGData& rpg) {
	FDEBUG("main", "Processing action " << _handler->getAction());

	SimpleCSP* csp = _handler->instantiate_csp(layer);

	// We do not need to take values that were already achieved in the previous layer into account.
	// TODO - This is not correct yet and needs further thought - e.g. instead of constraints Y not in "set of already achieved values",
	// TODO - the correct thing to do would be a constraint "Y in set of already achieved values OR Y = [w]^k"
// 	for ( ScopedEffect::cptr effect : action.getEffects() ) {
// 		addNoveltyConstraints(effect->getAffected(), layer, *csp);
// 	}

	bool locallyConsistent = csp->checkConsistency(); // This enforces propagation of constraints

	if (!locallyConsistent) {
		FDEBUG("main", "The action CSP is locally inconsistent");
		#ifdef FS0_DEBUG
		// MRJ: So we can check what planning variables' domains became empty
		_handler->print_csp( getDebugLogStream("main"), csp );
		#endif
	} else {
		if (true) {  // Solve the CSP completely
			_handler->compute_support(csp, actionIdx, rpg);
		} else { // Check only local consistency
			// TODO - Don't forget to delete the CSP in case of premature exit
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
