

#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/base.hxx>

namespace fs0 { namespace language { namespace fstrips {

std::ostream& operator<<(std::ostream &os, const LogicalElement& o) { return o.print(os, ProblemInfo::getInstance()); }

} } } // namespaces
