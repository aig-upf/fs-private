
#pragma once

#include <cstring>
#include <rapidjson/document.h>
#include <fs/core/utils/loader.hxx>
#include <fs/core/utils/component_factory.hxx>
#include <fs/core/fstrips/loader.hxx>
#include <fs/core/constraints/registry.hxx>


namespace fs0 { class Problem; }

/* Generate the whole planning problem */
inline fs0::Problem* generate(const rapidjson::Document& data, const std::string& data_dir) {
	fs0::LogicalComponentRegistry::set_instance( std::make_unique<fs0::LogicalComponentRegistry>());
	fs0::BaseComponentFactory factory;
	fs0::fstrips::LanguageJsonLoader::loadLanguageInfo(data);
	fs0::Loader::loadProblemInfo(data, data_dir, factory);
	return fs0::Loader::loadProblem(data);
}
