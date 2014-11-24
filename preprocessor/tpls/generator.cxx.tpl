
#include "generator.hxx"
#include "components.hxx"
#include <utils/loader.hxx>

namespace aptk { namespace core { namespace solver {

using aptk::core::utils::StateLoader;

void generate(const std::string& data_dir, aptk::core::Problem& problem, bool constrained) {

	/* Define the actions */
	StateLoader::loadGroundedActions(data_dir + "/actions.data", ComponentFactory::instantiateAction, problem);

	/* Define the initial state */
	problem.setInitialState(StateLoader::loadStateFromFile(data_dir + "/init.data"));

	/* Load the state constraints */
	if (constrained) {
	    StateLoader::loadConstraints(data_dir + "/constraints.data", problem);
    }

	/* Define the goal evaluator */
	StateLoader::loadGoalEvaluator(data_dir + "/goal.data", ComponentFactory::instantiateGoal, problem);
	
}

} } } // namespaces