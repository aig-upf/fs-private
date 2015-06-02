
#include "components.hxx"
#include <utils/loader.hxx>

std::unique_ptr<External> external;

const std::string IncrementAction::name_ = "increment";
const ActionSignature IncrementAction::signature_ = {2};
const std::string DecrementAction::name_ = "decrement";
const ActionSignature DecrementAction::signature_ = {2};

void generate(const std::string& data_dir, Problem& problem) {
	external = std::unique_ptr<External>(new External(problem.getProblemInfo(), data_dir));
	Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateConstraint, ComponentFactory::instantiateGoal, problem);
}