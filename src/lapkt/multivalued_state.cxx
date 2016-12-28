
#include <boost/functional/hash.hpp>

#include "multivalued_state.hxx"
#include <problem_info.hxx>
#include <atom.hxx>


namespace lapkt {


template <typename _ValueT>
std::size_t
MultivaluedState<_ValueT>::computeHash() const { return boost::hash_range(_values.cbegin(), _values.cend()); }


template <typename _ValueT>
std::ostream&
MultivaluedState<_ValueT>::print(std::ostream& os) const {
	const fs0::ProblemInfo& info = fs0::ProblemInfo::getInstance();
	os << "MultivaluedState";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) {
		if (info.getVariableGenericType(i) == fs0::ProblemInfo::ObjectType::BOOL) {
			if (_values.at(i) == 0) continue;
			
			// Print only those atoms which are true in the state
			os << info.getVariableName(i);
		} else {
			os << info.getVariableName(i) << "=" << info.getObjectName(i, _values.at(i));
		}
		if (i < _values.size() - 1) os << ", ";
	}
	os << "]";
	return os;
}



} // namespaces
