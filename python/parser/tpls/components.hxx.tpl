
#pragma once

#include <fs_types.hxx>
#include <utils/loader.hxx>
#include <lib/rapidjson/document.h>
#include "external.hxx"

#include <utils/component_factory.hxx>

namespace fs0 { class Problem; }

using namespace fs0;

$method_factories

/* Generate the whole planning problem */
inline Problem* generate(const rapidjson::Document& data, const std::string& data_dir) {
	ComponentFactory factory;
	ProblemInfo& info = Loader::loadProblemInfo(data, data_dir, factory);
	std::unique_ptr<External> external = std::unique_ptr<External>(new External(info, data_dir));
	external->registerComponents();
	info.set_external(std::move(external));
	auto problem = Loader::loadProblem(data);
	return problem;
}
