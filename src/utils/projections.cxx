
#include <cassert>
#include <utils/projections.hxx>
#include <actions.hxx>
#include <core_problem.hxx>

namespace aptk { namespace core {

ObjectIdxVector Projections::project(const State& s, const VariableIdxVector& scope) {
	ObjectIdxVector values;
	values.reserve(scope.size());
	for (VariableIdx idx:scope) {
		values.push_back(s.getValue(idx));
	}
	return values;
}

DomainMap Projections::projectToActionVariables(RelaxedState& state, const ApplicableEntity& action) {
	return projectCopy(state, action.getAllRelevantVariables());
}

DomainMap Projections::project(RelaxedState& state, const VariableIdxVector& scope) { //!  TODO - CHECK THIS IS APPLYING RVO??
	DomainMap projection;
	for (VariableIdx var:scope) {
		DomainPtr domain = state.getValues(var);
		projection.insert(std::make_pair(var, domain));
	}
	return projection;
}

DomainMap Projections::projectCopy(const RelaxedState& state, const VariableIdxVector& scope) {
	DomainMap projection;
	for (VariableIdx var:scope) {
		DomainPtr domain = std::make_shared<Domain>(*(state.getValues(var))); // We copy construct the whole domain
		projection.insert(std::make_pair(var, domain));
	}
	return projection;
}	

DomainVector Projections::project(const DomainMap& domains, const VariableIdxVector& scope) {
	DomainVector projection;
	projection.reserve(scope.size());
	for (VariableIdx var:scope) {
		projection.push_back(domains.at(var));
	}
	return projection;
}

void Projections::printDomains(const DomainMap& domains) {
		const auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
 	 	for (const auto& domain:domains) {
			std::cout << problemInfo->getVariableName(domain.first) << "={";
			for (auto objIdx:*(domain.second)) {
				std::cout << problemInfo->getObjectName(domain.first, objIdx) << ",";
			}
 	 		std::cout << "}" << std::endl;
		}
	}

} } // namespaces
