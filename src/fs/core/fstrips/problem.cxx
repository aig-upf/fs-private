

#include <fs/core/fstrips/problem.hxx>
#include <fs/core/utils/utils.hxx>

namespace fs0 { namespace fstrips {

Problem::Problem(const Problem& other) :
    name_(other.name_),
    domain_name_(other.domain_name_),
    schemas_(Utils::clone(other.schemas_)),
    init_(other.init_),
    goal_(other.goal_->clone())
{
}

std::ostream& Problem::print(std::ostream& os, const LanguageInfo& info) const {
    throw std::runtime_error("Unimplemented");
}
} } // namespaces
