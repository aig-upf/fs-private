
#pragma once

#include <cstring>
#include <rapidjson/document.h>
#include <fs/core/utils/loader.hxx>
#include <fs/core/utils/component_factory.hxx>
#include <fs/core/fstrips/loader.hxx>
#include <fs/core/constraints/registry.hxx>

#include "external.hxx"

namespace fs0 { class Problem; }

using namespace fs0;

$method_factories

/* Generate the whole planning problem */
inline Problem* generate(const rapidjson::Document& data, const std::string& data_dir) {
	fs0::LogicalComponentRegistry::set_instance( std::make_unique<fs0::LogicalComponentRegistry>());
	fs0::BaseComponentFactory factory;

	fs0::fstrips::LanguageJsonLoader::loadLanguageInfo(data);
	ProblemInfo& info = Loader::loadProblemInfo(data, data_dir, factory);

	std::unique_ptr<External> external = std::unique_ptr<External>(new External(info, data_dir));
	external->registerComponents();
	info.set_external(std::move(external));


	return fs0::Loader::loadProblem(data);
}
