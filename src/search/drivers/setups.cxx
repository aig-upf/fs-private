
#include <problem.hxx>
#include <actions/grounding.hxx>
#include <search/drivers/setups.hxx>
#include <search/drivers/validation.hxx>
#include <constraints/gecode/handlers/lifted_action_csp.hxx>
#include <models/ground_state_model.hxx>


namespace fs0 { namespace drivers {

LiftedStateModel 
GroundingSetup::fully_lifted_model(Problem& problem) {
	Validation::check_no_conditional_effects(problem);
	
	// We don't ground any action
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	return LiftedStateModel::build(problem);
}

GroundStateModel
GroundingSetup::fully_ground_model(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem); 
}

SimpleStateModel
GroundingSetup::fully_ground_simple_model(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return SimpleStateModel::build(problem); 
}

GroundStateModel
GroundingSetup::ground_search_lifted_heuristic(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem);
}

} } // namespaces
