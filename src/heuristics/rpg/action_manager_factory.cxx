
#include <cassert>
#include <iosfwd>

#include <heuristics/rpg/action_manager_factory.hxx>
#include <constraints/gecode/action_manager.hxx>
#include <constraints/direct/action_manager.hxx>
#include <utils/cartesian_product.hxx>
#include <heuristics/rpg_data.hxx>
#include <constraints/filtering.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>

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

	bool complexPreconditions = actionHasHigherArityPreconditions(action);
	bool complexEffects = actionHasHigherArityEffects(action);
	BaseActionManager* manager = nullptr;

	if (manager_t == Config::ActionManagerType::Gecode || complexPreconditions || complexEffects) {
		manager = gecode::GecodeActionManager::create(action);
		FDEBUG("main", "Generated CSP for action " << action << std::endl <<  *manager << std::endl);
	} else {
		manager = DirectActionManager::create(action);
		FDEBUG("main", "Generated a DirectActionManager for action " << action << std::endl <<  *manager << std::endl);
	}

	return manager;
}


bool ActionManagerFactory::actionHasHigherArityPreconditions(const GroundAction& action) {
	bool needs_generic_manager = false;
	for (const AtomicFormula::cptr constraint:action.getConditions()) {
		unsigned arity = constraint->getScope().size();
		if (arity == 0) {
			throw std::runtime_error("Static applicability procedure that should have been detected in compilation time");
		}
		else if(arity == 1) {
// 			if (!dynamic_cast<UnaryParametrizedScopedConstraint*>(constraint) || constraint->filteringType() != FilteringType::Unary) {
// 				throw std::runtime_error("Cannot handle this type of unary constraint in action preconditions.");
// 			}
		} else {
			needs_generic_manager = true;
		}
	}
	return needs_generic_manager;
}

bool ActionManagerFactory::actionHasHigherArityEffects(const GroundAction& action) {
	for (const ActionEffect::cptr effect:action.getEffects()) {
		if (effect->scope.size() > 1) {
			return true;
		}
	}
	return false;
}




} // namespaces
