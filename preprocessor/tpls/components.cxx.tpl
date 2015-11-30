
#include "components.hxx"
#include <utils/loader.hxx>

std::unique_ptr<External> external;

void generate(const rapidjson::Document& data, const std::string& data_dir) {
	ComponentFactory factory;
	Loader::loadProblemInfo(data, factory);
	external = std::unique_ptr<External>(new External(data_dir));
	external->registerComponents();
	Loader::loadProblem(data);
}