
#include "components.hxx"


namespace aptk { namespace core { namespace solver {

std::unique_ptr<External> external;

$action_code

void generate(const std::string& data_dir, aptk::core::Problem& problem) {
	external = std::unique_ptr<External>(new External(data_dir));
	utils::Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateGoal, problem);
}

} } } // namespaces