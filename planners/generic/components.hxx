
#pragma once

#include <cstring>
#include <lib/rapidjson/document.h>

namespace fs0 { class Problem; }

/* Generate the whole planning problem */
fs0::Problem* generate(const rapidjson::Document& data, const std::string& data_dir);
