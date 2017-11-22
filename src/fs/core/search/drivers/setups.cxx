
#include <fs/core/problem.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/drivers/validation.hxx>
#ifdef FS_HYBRID
#include <fs/hybrid/dynamics/wait_action.hxx>
#endif

namespace fs0 { namespace drivers {

LiftedStateModel
GroundingSetup::fully_lifted_model(Problem& problem) {
	Validation::check_no_conditional_effects(problem);

	// We don't ground any action
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	#ifdef FS_HYBRID
	if ( problem.requires_handling_continuous_change() ) {
		problem.add_wait_action( dynamics::WaitAction::create(problem) );
	}
	#endif
	return LiftedStateModel::build(problem);
}

GroundStateModel
GroundingSetup::fully_ground_model(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	#ifdef FS_HYBRID
	if ( problem.requires_handling_continuous_change() ) {
		problem.add_wait_action( dynamics::WaitAction::create(problem) );
	}
	#endif
	return GroundStateModel(problem);
}

SimpleStateModel
GroundingSetup::fully_ground_simple_model(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	#ifdef FS_HYBRID
	if ( problem.requires_handling_continuous_change() ) {
		// TODO UNCOMMENT FOR BETA7
		problem.add_wait_action( dynamics::WaitAction::create(problem) );
	}
	#endif
	return SimpleStateModel::build(problem);
}

GroundStateModel
GroundingSetup::ground_search_lifted_heuristic(Problem& problem) {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	problem.setPartiallyGroundedActions(ActionGrounder::fully_lifted(problem.getActionData(), ProblemInfo::getInstance()));
	//! Determine if computing successor states requires to handle continuous change
	#ifdef FS_HYBRID
	if ( problem.requires_handling_continuous_change() ) {
		// TODO UNCOMMENT FOR BETA7
		problem.add_wait_action( dynamics::WaitAction::create(problem) );
	}
	#endif
	return GroundStateModel(problem);
}

} } // namespaces
