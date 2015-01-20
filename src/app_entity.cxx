
#include <app_entity.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <limits>
#include <utils/utils.hxx>


namespace fs0 {


AppEntity::AppEntity(const std::vector<ScopedConstraint const *>& constraints) :
	_constraints(constraints), _allRelevantVars(extractRelevantVariables(constraints))
{};

VariableIdxVector AppEntity::extractRelevantVariables(const std::vector<ScopedConstraint const *>& constraints) {
	boost::container::flat_set<unsigned> unique;
	for (const ScopedConstraint* constraint:constraints) {
		const VariableIdxVector& scope = constraint->getScope();
		unique.insert(scope.begin(), scope.end());
	}
	return VariableIdxVector(unique.cbegin(), unique.cend());
}

AppEntity::~AppEntity() {
	for (const ScopedConstraint* constraint:_constraints) delete constraint;
};

} // namespaces
