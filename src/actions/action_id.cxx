
#include <actions/action_id.hxx>
#include <actions/ground_action.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>
#include <utils/printers/actions.hxx>
#include <boost/functional/hash.hpp>

namespace fs0 {

const ActionID* ActionID::make_invalid() {
	return new PlainActionID(std::numeric_limits<unsigned int>::max());
}

const LiftedActionID LiftedActionID::invalid_action_id = LiftedActionID(std::numeric_limits<unsigned int>::max(), std::vector<ObjectIdx>());

LiftedActionID::LiftedActionID(unsigned schema_id, const std::vector<ObjectIdx>& binding)
	: ActionID(schema_id), _binding(binding)
{}

LiftedActionID::LiftedActionID(unsigned schema_id, const Binding& binding) 
	: LiftedActionID(schema_id, binding.get_full_binding())
{}

LiftedActionID::LiftedActionID(unsigned schema_id, std::vector<ObjectIdx>&& binding)
	: ActionID(schema_id), _binding(std::move(binding))
{}

bool LiftedActionID::operator==(const ActionID& rhs) const {
	auto derived = dynamic_cast<const LiftedActionID*>(&rhs);
	if(!derived) return false;
	return  _id == derived->_id && _binding == derived->_binding;
	
}

bool LiftedActionID::operator< (const ActionID& rhs) const {
	if (typeid(*this).before(typeid(rhs))) return true;
	if (typeid(rhs).before(typeid(*this))) return false;
	
	// otherwise both typeids are equal
	auto derived = dynamic_cast<const LiftedActionID*>(&rhs);
	assert(derived);
	return  _id < derived->_id || ( _id == derived->_id && _binding < derived->_binding);
}

bool PlainActionID::operator==(const ActionID& rhs) const {
	auto derived = dynamic_cast<const PlainActionID*>(&rhs);
	if(!derived) return false;
	return  _id == derived->_id;
}

bool PlainActionID::operator< (const ActionID& rhs) const {
	if (typeid(*this).before(typeid(rhs))) return true;
	if (typeid(rhs).before(typeid(*this))) return false;
	
	// otherwise both typeids are equal
	auto derived = dynamic_cast<const PlainActionID*>(&rhs);
	assert(derived);
	return  _id < derived->_id;
}

std::size_t LiftedActionID::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _id);
	boost::hash_combine(hash, boost::hash_range(_binding.begin(), _binding.end()));
	return hash;
}

std::size_t PlainActionID::hash_code() const {
	std::size_t hash = 0;
	boost::hash_combine(hash, typeid(*this).hash_code());
	boost::hash_combine(hash, _id);
	return hash;
}

std::ostream& LiftedActionID::print(std::ostream& os) const {
	if (is_valid()) {
		ActionSchema::cptr schema_ = schema();
		os << schema_->getName() << "(" << print::binding(getBinding(), schema_->getSignature()) << ")";
	} else {
		os << "[INVALID-ACTION]";
	}	
	return os;
}

std::ostream& PlainActionID::print(std::ostream& os) const {
	if (is_valid()) {
		const GroundAction* action = Problem::getInstance().getGroundActions()[_id];
		os << action->fullname();
	} else {
		os << "[INVALID-ACTION]";
	}
	return os;
}

const ActionSchema* LiftedActionID::schema() const { return Problem::getInstance().getActionSchemata()[_id]; }

GroundAction* LiftedActionID::generate() const {
	const ProblemInfo& info = Problem::getInfo();
	return schema()->bind(Binding(_binding), info);
}


} // namespaces
