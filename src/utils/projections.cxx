
#include <cassert>
#include <utils/projections.hxx>
#include <app_entity.hxx>
#include <problem.hxx>

namespace fs0 {

ObjectIdxVector Projections::project(const State& s, const VariableIdxVector& scope) {
	ObjectIdxVector values;
	values.reserve(scope.size());
	for (VariableIdx idx:scope) {
		values.push_back(s.getValue(idx));
	}
	return values;
}

DomainMap Projections::projectToActionVariables(RelaxedState& state, const Action& action) {
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

DomainMap Projections::clone(const DomainMap& domains) {
	DomainMap clone;
	for (const auto& domain:domains) {
		clone.insert(std::make_pair(domain.first, std::make_shared<Domain>(*(domain.second))));
	}
	return clone;
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

void Projections::printDomains(const DomainVector& domains) {
	const auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	for (unsigned i = 0; i < domains.size(); ++i) {
		const Domain& domain = *domains[i];
		std::cout << "variable #" << i << " ={";
		for (auto objIdx:domain) {
			std::cout << objIdx << ",";
		}
		std::cout << "}" << std::endl;
	}
}

} // namespaces
