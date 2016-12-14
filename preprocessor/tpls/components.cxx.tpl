
#include "components.hxx"
#include <utils/loader.hxx>

Problem* generate(const rapidjson::Document& data, const std::string& data_dir) {
	ComponentFactory factory;
	ProblemInfo& info = Loader::loadProblemInfo(data, data_dir, factory);
	std::unique_ptr<External> external = std::unique_ptr<External>(new External(info, data_dir));
	external->registerComponents();
	info.set_external(std::move(external));
	auto problem = Loader::loadProblem(data, nullptr);
	return problem;
}