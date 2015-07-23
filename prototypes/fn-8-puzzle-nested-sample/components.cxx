
#include "components.hxx"
#include <utils/loader.hxx>

std::unique_ptr<External> external;

const std::string MoveAction::name_ = "move";
const ActionSignature MoveAction::signature_ = {2,5,5};

void generate(const rapidjson::Document& data, const std::string& data_dir, Problem& problem) {
	external = std::unique_ptr<External>(new External(problem.getProblemInfo(), data_dir));
	ComponentFactory factory;
	Loader::loadProblem(data, factory, problem);
}