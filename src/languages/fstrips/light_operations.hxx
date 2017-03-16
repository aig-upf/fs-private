

#pragma once

#include <languages/fstrips/light.hxx>

namespace fs0 { namespace lang { namespace fstrips {

//! Returns true if some parameter of the action has a type with no associated object
bool has_empty_parameter(const ActionSchema& schema);

} } } // namespaces

