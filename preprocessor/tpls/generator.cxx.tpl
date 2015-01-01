
#include "generator.hxx"
#include "components.hxx"
#include <utils/loader.hxx>

namespace aptk { namespace core { namespace solver {

using aptk::core::utils::StateLoader;

void generate(const std::string& data_dir, aptk::core::Problem& problem) {

	/* Define the actions */
	StateLoader::loadGroundedActions(data_dir + "/actions.data", ComponentFactory::instantiateAction, problem);

	/* Define the initial state */
	problem.setInitialState(StateLoader::loadStateFromFile(data_dir + "/init.data"));

	/* Load the state and goal constraints */
	StateLoader::loadConstraints(data_dir + "/constraints.data", problem, false);
	StateLoader::loadConstraints(data_dir + "/goal-constraints.data", problem, true);

	/* Generate goal constraints from the goal evaluator */
	StateLoader::generateGoalConstraints(data_dir + "/goal.data", ComponentFactory::instantiateGoal, problem);
	
	problem.createConstraintManager();
}

} } } // namespaces