
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <fs0_types.hxx>
#include <utils/serializer.hxx>

namespace fs0 { namespace asp { class LPHandler; }}

using namespace fs0;

/*********************************************/
/* The static data                           */
/*********************************************/
class ExternalBase  {
protected:
    ${data_declarations}

public:
    ExternalBase(const std::string& data_dir) ${data_initialization}
    {}

    ${data_accessors}

    //! By default, we register no component
    virtual void registerComponents() const {}

    //! To be subclassed in case the particular problem has a ASP handler
    virtual asp::LPHandler* get_asp_handler() const { return nullptr; }
};

