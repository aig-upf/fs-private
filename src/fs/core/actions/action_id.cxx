
#include <fs/core/actions/action_id.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <boost/functional/hash.hpp>


namespace fs0 {


unsigned PlainActionID::id() const { return _action->getId(); }

bool PlainActionID::operator==(const ActionID& rhs) const {
	auto derived = dynamic_cast<const PlainActionID*>(&rhs);
	if(!derived) return false;
	return  id() == derived->id();
}


std::size_t PlainActionID::hash() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, id());
	return hash;
}


std::ostream& PlainActionID::print(std::ostream& os) const {
	(_action? os << *_action : os << "INVALID-ACTION");
	return os;
}


} // namespaces
