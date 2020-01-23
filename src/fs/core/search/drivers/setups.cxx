
#include <fs/core/problem.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/drivers/validation.hxx>

namespace fs0::drivers {

CSPLiftedStateModel
GroundingSetup::fully_lifted_model(Problem& problem) {
	Validation::check_no_conditional_effects(problem);

	// We don't ground any action
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	return CSPLiftedStateModel::build(problem);
}

GroundStateModel
GroundingSetup::fully_ground_model(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	return GroundStateModel(problem);
}

SimpleStateModel
GroundingSetup::fully_ground_simple_model(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	return SimpleStateModel::build(problem);
}

GroundStateModel
GroundingSetup::ground_search_lifted_heuristic(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	return GroundStateModel(problem);
}

SDDLiftedStateModel
GroundingSetup::sdd_lifted_model(Problem& problem) {
    Validation::check_no_conditional_effects(problem);

    // We don't ground any action
    problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));

    return SDDLiftedStateModel::build(problem);
}

} // namespaces
