
#pragma once

#include <vector>
#include <ostream>
#include <fs_types.hxx>
#include <aptk2/heuristics/novelty/tuples.hxx>

namespace fs0 { namespace print {

class feature_set {
	protected:
		std::vector<aptk::VariableIndex>& _varnames;
		
		std::vector<aptk::ValueIndex>& _values;
		
	public:
		feature_set(std::vector<aptk::VariableIndex>& varnames, std::vector<aptk::ValueIndex>& values) : _varnames(varnames), _values(values) {}
		
		friend std::ostream& operator<<(std::ostream &os, const feature_set& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
