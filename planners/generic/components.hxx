
#pragma once

#include <cstring>
#include <lib/rapidjson/document.h>

namespace fs0 { class Problem; }
namespace fs0 { namespace asp { class LPHandler; }}

/* Generate the whole planning problem */
fs0::Problem* generate(const rapidjson::Document& data, const std::string& data_dir);


class ExternalBase  {
public:
    ExternalBase(const std::string& data_dir) 
    {}

    //! By default, we register no component
    virtual void registerComponents() const {}

    //! To be subclassed in case the particular problem has a ASP handler
    virtual fs0::asp::LPHandler* get_asp_handler() const { return nullptr; }
};
