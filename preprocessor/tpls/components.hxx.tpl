
#pragma once

#include <fs0_types.hxx>
#include <problem.hxx>
#include "external.hxx"

#include <component_factory.hxx>

using namespace fs0;

extern std::unique_ptr<External> external;

$method_factories

/* Generate the whole planning problem */
void generate(const rapidjson::Document& data, const std::string& data_dir, Problem& problem);
