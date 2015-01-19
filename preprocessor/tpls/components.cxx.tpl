
#include "components.hxx"


namespace aptk { namespace core { namespace solver {

std::unique_ptr<External> external = std::unique_ptr<External>(new External());

$action_code

void generate(const std::string& data_dir, aptk::core::Problem& problem) {
	utils::Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateGoal, problem);
}

} } } // namespaces