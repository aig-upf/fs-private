
#pragma once

#include <vector>
#include <ostream>
#include <fs0_types.hxx>

namespace fs0 { namespace print {

class Helper {
	public:
		static const std::vector<std::string> name_variables(const std::vector<VariableIdx>& variables);
};



} } // namespaces
