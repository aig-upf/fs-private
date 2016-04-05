
#include <actions/action_id.hxx>
#include <actions/actions.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <boost/functional/hash.hpp>

namespace fs0 {

const LiftedActionID LiftedActionID::invalid_action_id = LiftedActionID(nullptr, std::vector<ObjectIdx>());

LiftedActionID::LiftedActionID(const BaseAction* action, const std::vector<ObjectIdx>& binding)
	: _action(action), _binding(binding)
{}

LiftedActionID::LiftedActionID(const BaseAction* action, const Binding& binding) 
	: LiftedActionID(action, binding.get_full_binding())
{}

LiftedActionID::LiftedActionID(const BaseAction* action, std::vector<ObjectIdx>&& binding)
	: _action(action), _binding(std::move(binding))
{}

bool LiftedActionID::operator==(const ActionID& rhs) const {
	auto derived = dynamic_cast<const LiftedActionID*>(&rhs);
	if(!derived) return false;
	return  id() == derived->id() && _binding == derived->_binding;
}

unsigned LiftedActionID::id() const { return _action->getId(); }

unsigned PlainActionID::id() const { return _action->getId(); }

bool PlainActionID::operator==(const ActionID& rhs) const {
	auto derived = dynamic_cast<const PlainActionID*>(&rhs);
	if(!derived) return false;
	return  id() == derived->id();
}

std::size_t LiftedActionID::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, id());
	boost::hash_combine(hash, boost::hash_range(_binding.begin(), _binding.end()));
	return hash;
}

std::size_t PlainActionID::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, id());
	return hash;
}

std::ostream& LiftedActionID::print(std::ostream& os) const {
	os << _action->fullname();
	return os;
}

std::ostream& PlainActionID::print(std::ostream& os) const {
	os << _action->fullname();
	return os;
}


GroundAction* LiftedActionID::generate() const { // TODO This dynamic_cast here is very ugly
	if (const GroundAction* ground = dynamic_cast<const GroundAction*>(_action)) return new GroundAction(*ground);
	const ActionSchema* lifted = dynamic_cast<const ActionSchema*>(_action);
	assert(lifted);
	const ProblemInfo& info = Problem::getInfo();
	return lifted->bind(Binding(_binding), info);
}

} // namespaces
