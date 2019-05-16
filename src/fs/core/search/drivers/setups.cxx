
#include <fs/core/problem.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/search/drivers/validation.hxx>

namespace fs0 { namespace drivers {

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

} } // namespaces
