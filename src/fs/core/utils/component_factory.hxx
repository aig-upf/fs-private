
#pragma once

#include <fs/core/fs_types.hxx>

namespace fs0 {

class ProblemInfo;

//! An interface for the concrete instance generators
class BaseComponentFactory {
public:
	//! To be subclassed if necessary
    virtual std::map<std::string, Function> instantiateFunctions(const ProblemInfo& info) const { return {}; }
};

} // namespaces