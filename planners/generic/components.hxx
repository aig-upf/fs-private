
#pragma once

#include <cstring>
#include <lib/rapidjson/document.h>
#include <utils/loader.hxx>
#include <utils/component_factory.hxx>


namespace fs0 { class Problem; }

/* Generate the whole planning problem */
inline fs0::Problem* generate(const rapidjson::Document& data, const std::string& data_dir) {
	fs0::BaseComponentFactory factory;
	fs0::Loader::loadProblemInfo(data, data_dir, factory);
	return fs0::Loader::loadProblem(data);
}