
#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

namespace aptk { namespace core { namespace solver {

/*********************************************/
/* The static data                           */
/*********************************************/
class ExternalBase  {
protected:
    ${data_declarations}

public:
    void bootstrap() {}

    ${data_accessors}
};


} } } // namespaces
