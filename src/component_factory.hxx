
#pragma once

#include <fs0_types.hxx>

namespace fs0 {

//! An interface for the concrete instance generators
class BaseComponentFactory {
public:
    virtual std::map<std::string, Function> instantiateFunctions() const = 0;
};

} // namespaces