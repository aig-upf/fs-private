
#include "components.hxx"


namespace aptk { namespace core { namespace solver {

${data_initializations}

$action_code

void generate(const std::string& data_dir, aptk::core::Problem& problem) {
	utils::Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateGoal, problem);
}

} } } // namespaces