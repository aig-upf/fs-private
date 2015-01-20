
#include <app_entity.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>


namespace fs0 {

const ActionIdx Action::INVALID = std::numeric_limits<unsigned int>::max();

Action::Action(const ObjectIdxVector& binding, const ScopedConstraint::vcptr& constraints, const ScopedEffect::vcptr& effects) :
	_binding(binding), _constraints(constraints), _effects(effects), _allRelevantVars(extractRelevantVariables())
{};

VariableIdxVector Action::extractRelevantVariables() {
	boost::container::flat_set<unsigned> unique;
	for (ScopedConstraint::cptr constraint:_constraints) {
		const VariableIdxVector& scope = constraint->getScope();
		unique.insert(scope.begin(), scope.end());
	}
	for (ScopedEffect::cptr effect:_effects) {
		const VariableIdxVector& scope = effect->getScope();
		unique.insert(scope.begin(), scope.end());
	}	
	return VariableIdxVector(unique.cbegin(), unique.cend());
}

Action::~Action() {
	for (const ScopedConstraint* constraint:_constraints) delete constraint;
};

std::ostream& Action::print(std::ostream& os) const {
	auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << getName() << "(";
	for(auto obj:_binding) {
		os << problemInfo->getCustomObjectName(obj) << ", ";
	}
	os << ")";
	return os;
}

} // namespaces
