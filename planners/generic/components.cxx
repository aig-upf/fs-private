
#include "components.hxx"
#include <utils/loader.hxx>
#include <component_factory.hxx>

std::unique_ptr<ExternalBase> external;

fs0::Problem* generate(const rapidjson::Document& data, const std::string& data_dir) {
	fs0::BaseComponentFactory factory;
	fs0::Loader::loadProblemInfo(data, factory);
	external = std::unique_ptr<ExternalBase>(new ExternalBase(data_dir));
	external->registerComponents();
	return fs0::Loader::loadProblem(data, external->get_asp_handler());
}