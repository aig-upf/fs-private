
#include <search/drivers/validation.hxx>
#include <problem.hxx>
#include <languages/fstrips/effects.hxx>
#include <languages/fstrips/formulae.hxx>
#include <actions/actions.hxx>

namespace fs0 { namespace drivers {

void Validation::check_no_conditional_effects(const Problem& problem) {
	const std::vector<const ActionData*>& action_data = problem.getActionData();
	for (const ActionData* action:action_data) {
		for (const fs::ActionEffect* effect:action->getEffects()) {
			if (!effect->condition()->is_tautology()) throw std::runtime_error("Conditional Effects not supported in this search driver");
		}
	}
}


} } // namespaces
