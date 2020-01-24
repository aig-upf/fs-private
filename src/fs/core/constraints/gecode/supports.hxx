
#pragma once

#include <fs/core/fs_types.hxx>

namespace fs0 { class AtomIndex; }

namespace fs0 { namespace gecode {

class GecodeSpace; class CSPTranslator;

//! Some helper methods related to the extraction of supports from Gecode models
class Supports {
public:
	static std::vector<AtomIdx> extract_support(const GecodeSpace* solution, const CSPTranslator& translator, const std::vector<std::pair<unsigned, std::vector<unsigned>>>& tuple_indexes, const std::vector<AtomIdx>& necessary_tuples);
};

} } // namespaces