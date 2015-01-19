
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include <utils/serializer.hxx>

namespace aptk { namespace core { namespace solver {

/*********************************************/
/* The static data                           */
/*********************************************/
class ExternalBase  {
protected:
    ${data_declarations}

public:
    ExternalBase(const std::string& data_dir)
        : ${data_initialization}
    {}


    ${data_accessors}
};


} } } // namespaces
