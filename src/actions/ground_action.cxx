
#include <actions/ground_action.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>
#include <utils/printers/binding.hxx>


namespace fs0 {

const ActionIdx GroundAction::invalid_action_id = std::numeric_limits<unsigned int>::max();

GroundAction::GroundAction(ActionSchema::cptr schema, const ObjectIdxVector& binding, const std::vector<AtomicFormula::cptr>& conditions, const std::vector<ActionEffect::cptr>& effects)
	: _schema(schema), _binding(binding), _conditions(conditions), _effects(effects),
	  _scope(extractScope()),
	  _allRelevant(extractRelevantVariables()),
	  _allAffected(extractAffectedVariables())
{}


GroundAction::~GroundAction() {
	for (const auto pointer:_conditions) delete pointer;
	for (const auto pointer:_effects) delete pointer;
}

VariableIdxVector GroundAction::extractScope() const {
	std::set<VariableIdx> unique;
	
	for (const AtomicFormula::cptr formula:_conditions) {
		VariableIdxVector scope = formula->getScope();
		unique.insert(scope.begin(), scope.end());
	}
	
	return VariableIdxVector(unique.cbegin(), unique.cend());
}

VariableIdxVector GroundAction::extractRelevantVariables() const {
	std::set<VariableIdx> unique(_scope.begin(), _scope.end());
	for (ActionEffect::cptr effect:_effects) effect->computeScope(unique);
	return VariableIdxVector(unique.cbegin(), unique.cend());
}

VariableIdxVector GroundAction::extractAffectedVariables() const {
	std::set<VariableIdx> unique;
	for (ActionEffect::cptr effect:_effects) effect->computeAffected(unique);
	return VariableIdxVector(unique.cbegin(), unique.cend());
}

std::ostream& GroundAction::print(std::ostream& os) const {
	os << getName() << "(" << print::binding(_binding, _schema->getSignature()) << ")";
	
	os << "Preconditions:" << std::endl;
	for (auto elem:_conditions) os << "\t" << *elem << std::endl;
	
	os << "Effects:" << std::endl;
	for (auto elem:_effects) os << "\t" << *elem << std::endl;
		
	return os;
}


} // namespaces
