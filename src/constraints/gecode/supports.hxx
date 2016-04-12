
#pragma once

#include <fs_types.hxx>

namespace fs0 { class TupleIndex; }

namespace fs0 { namespace gecode {

class SimpleCSP; class GecodeCSPVariableTranslator;

//! Some helper methods related to the extraction of supports from Gecode models
class Supports {
public:
	static std::vector<TupleIdx> extract_support(const SimpleCSP* solution, const GecodeCSPVariableTranslator& translator, const std::vector<std::pair<unsigned, std::vector<unsigned>>>& tuple_indexes);
};

} } // namespaces