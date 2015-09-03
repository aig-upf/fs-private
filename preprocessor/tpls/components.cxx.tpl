
#include "components.hxx"
#include <utils/loader.hxx>

std::unique_ptr<External> external;

std::unique_ptr<Problem> generate(const rapidjson::Document& data, const std::string& data_dir) {
	external = std::unique_ptr<External>(new External(data_dir));
	external->registerComponents();
	ComponentFactory factory;
	Problem* _problem = new Problem;
	Loader::loadProblem(data, factory, *_problem);
	return std::unique_ptr<Problem>(_problem);
}