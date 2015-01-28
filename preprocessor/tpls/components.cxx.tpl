
#include "components.hxx"
#include <utils/loader.hxx>

std::unique_ptr<External> external;

$action_code

void generate(const std::string& data_dir, Problem& problem) {
	external = std::unique_ptr<External>(new External(problem.getProblemInfo(), data_dir));
	Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateConstraint, ComponentFactory::instantiateGoal, problem);
}