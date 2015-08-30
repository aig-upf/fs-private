
#include <cassert>
#include <iosfwd>

#include <heuristics/rpg/action_manager_factory.hxx>
#include <constraints/gecode/action_manager.hxx>
#include <constraints/direct/action_manager.hxx>
#include <heuristics/rpg_data.hxx>
#include <constraints/filtering.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/scopes.hxx>

namespace fs0 {


std::vector<std::shared_ptr<BaseActionManager>> ActionManagerFactory::create(const std::vector<GroundAction::cptr>& actions) {
	std::vector<std::shared_ptr<BaseActionManager>> managers;
	managers.reserve(actions.size());
	for (const auto action:actions) {
		managers.push_back(std::shared_ptr<BaseActionManager>(create(*action)));
	}
	return managers;
}

BaseActionManager* ActionManagerFactory::create(const GroundAction& action) {
	const Config::ActionManagerType manager_t = Config::instance().getActionManagerType();

	BaseActionManager* manager = nullptr;
	if (manager_t == Config::ActionManagerType::Gecode || gecodeManagerNeeded(action)) {
		manager = gecode::GecodeActionManager::create(action);
		FDEBUG("main", "Generated CSP for action " << action << std::endl <<  *manager << std::endl);
	} else {
		manager = DirectActionManager::create(action);
		FDEBUG("main", "Generated a DirectActionManager for action " << action << std::endl <<  *manager << std::endl);
	}

	return manager;
}


bool ActionManagerFactory::gecodeManagerNeeded(const GroundAction& action) {
	
	for (const AtomicFormula::cptr condition:action.getConditions()) {
		if (condition->nestedness() > 0) return true;
		
		unsigned arity = ScopeUtils::computeDirectScope(condition).size();
		if (arity == 0) throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
		else if(arity > 1) return true;
	}
	
	for (const ActionEffect::cptr effect:action.getEffects()) {
		if (effect->lhs()->nestedness() > 0 || effect->rhs()->nestedness() > 0 || ScopeUtils::computeDirectScope(effect).size() > 1) return true;
	}
	
	return false;
}





} // namespaces
