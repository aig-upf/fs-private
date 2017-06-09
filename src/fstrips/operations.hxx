

#pragma once

#include <fstrips/language.hxx>

namespace fs0 { namespace fstrips {

//! Returns true if some parameter of the action has a type with no associated object
bool has_empty_parameter(const ActionSchema& schema);

} } // namespaces

