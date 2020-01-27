
#include <fs/core/search/drivers/validation.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/languages/fstrips/effects.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/actions/actions.hxx>

namespace fs0::drivers {

void Validation::check_no_conditional_effects(const Problem& problem) {
	const std::vector<const ActionData*>& action_data = problem.getActionData();
	for (const ActionData* action:action_data) {
		for (const fs::ActionEffect* effect:action->getEffects()) {
			if (!effect->condition()->is_tautology()) throw std::runtime_error("Conditional Effects not supported in this search driver");
		}
	}
}


} // namespaces
