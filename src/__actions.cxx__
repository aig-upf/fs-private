
#include <actions.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>


namespace fs0 {


//! Note that the only relevant variables for applicable entities are the variables relevant to the different applicability procedures
ApplicableEntity::ApplicableEntity(const std::vector<VariableIdxVector>& appRelevantVars) :
	_appRelevantVars(appRelevantVars), _allRelevantVars(Utils::unique(Utils::flatten(appRelevantVars)))
{};

ApplicableEntity::ApplicableEntity(const std::vector<VariableIdxVector>& appRelevantVars, VariableIdxVector&& allRelevantVars) :
	_appRelevantVars(appRelevantVars), _allRelevantVars(allRelevantVars)
{};


std::ostream& ApplicableEntity::print(std::ostream& os) const {
	// TODO
	os << "ApplicableEntity[...]";
	return os;
}

	
const ActionIdx CoreAction::INVALID_ACTION = std::numeric_limits<unsigned int>::max();
	
//! Note that the relevant variables for actions are the variables relevant to the applicability procedures
//! plus those relevant to the effect procedures
CoreAction::CoreAction(const ObjectIdxVector& binding,
			const ObjectIdxVector& derived,
			const std::vector<VariableIdxVector>& appRelevantVars,
			const std::vector<VariableIdxVector>& effRelevantVars,
			const std::vector<VariableIdxVector>& effAffectedVars
) : ApplicableEntity(appRelevantVars, Utils::unique(Utils::merge(Utils::flatten(appRelevantVars), Utils::flatten(effRelevantVars)))),
	_binding(binding),
	_derived(derived),
	_effRelevantVars(effRelevantVars),
	_effAffectedVars(effAffectedVars)
{
	// Both vectors must have the same size, which is equal to the number of effect procedures.
	assert(_effRelevantVars.size() == _effAffectedVars.size());
};
	
std::ostream& CoreAction::print(std::ostream& os) const {
	auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << getName() << "(";
	for(auto obj:_binding) {
		os << problemInfo->getCustomObjectName(obj) << ", ";
	}
	os << ")";
// 	os << "\n\t" << "APPL: " << *_applicabilityFormula;
	return os;
}

} // namespaces
