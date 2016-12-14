
#pragma once

#include <vector>
#include <ostream>
#include <fs_types.hxx>

namespace fs0 { namespace bfws { class RelevantAtomSet; } }

namespace fs0 { namespace print {

class relevant_atomset {
	protected:
		const bfws::RelevantAtomSet& _set;
		
	public:
		relevant_atomset(const bfws::RelevantAtomSet& set) : _set(set) {}
		
		friend std::ostream& operator<<(std::ostream &os, const relevant_atomset& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};


} } // namespaces
