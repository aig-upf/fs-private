
#include <boost/functional/hash.hpp>

#include <fs/core/atom.hxx>
#include <fs/core/problem_info.hxx>

namespace fs0 {

std::ostream& Atom::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = ProblemInfo::getInstance();
	os << "[" << problemInfo.getVariableName(_variable) << "=" << problemInfo.object_name(_value) << "]";
	return os;
}

std::size_t Atom::hash() const {
        std::size_t hash = 0;
        boost::hash_combine(hash, _variable);
        boost::hash_combine(hash, _value);
        return hash;
}

} // namespaces
