
#pragma once

#include <fs_types.hxx>
#include <lib/rapidjson/document.h>
#include "external.hxx"

#include <component_factory.hxx>

namespace fs0 { class Problem; }

using namespace fs0;

extern std::unique_ptr<External> external;

$method_factories

/* Generate the whole planning problem */
Problem* generate(const rapidjson::Document& data, const std::string& data_dir);
