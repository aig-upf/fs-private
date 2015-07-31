
#include <problem.hxx>
#include <sstream>

#include <utils/logging.hxx>
#include "heuristics/rpg/action_manager_factory.hxx"

namespace fs0 {

const Problem* Problem::_instance = 0;

Problem::Problem(const rapidjson::Document& data) :
	_problemInfo(data),
	_stateConstraints(),
	_goalConditions()
{
}

Problem::~Problem() {
	for (const auto pointer:_schemata) delete pointer;
	for (const auto pointer:_ground) delete pointer;
	for (const auto pointer:_stateConstraints) delete pointer;
	for (const auto pointer:_goalConditions) delete pointer;
}


void Problem::bootstrap() {
	// Safety check
	if (_goalConditions.empty()) {
		throw std::runtime_error("No goal specification detected. The problem is probably being bootstrapped before having been fully initialized with the per-instance generate() procedure"); 
	}
	
// 	gecode::registerTranslators();
	
	// Add the necessary (unary) constraints to keep the appropriate values, if any, within their bounds.
// 	addDomainBoundConstraints();

	// Compile the constraints if necessary
// 	compileConstraints();
}

/*
//! For every action, if any of its effects affects a bounded-domain variable, we place
//! an "automatic" "bounds constraint" upon the variables which are relevant to the effect,
//! so that they won't generate an out-of-bounds value.
void Problem::addDomainBoundConstraints() {
	unsigned num_bconstraints = 0;

	std::cout << "Generating unary constraints for bounded domains..." << std::endl;

	for (Action::ptr action:_actions) {

		for (const ScopedEffect::cptr effect:action->getEffects()) {
			VariableIdx affected = effect->getAffected();
			if (!_problemInfo.hasVariableBoundedDomain(affected)) continue;

			// We process the creation of the bound-constraint differently  for each arity
			unsigned arity = effect->getScope().size();
			if (arity == 0) {
				if (!_problemInfo.checkValueIsValid(effect->apply())) {
					std::stringstream buffer;
					buffer << "Error: 0-ary effect '" << effect << "' of action ";
					buffer << action->getName() << " produces out-of-bounds variable values" << std::endl;
					throw std::runtime_error(buffer.str());
				}
			} else if (arity == 1) {
				const auto* casted_effect = dynamic_cast<const UnaryScopedEffect *>(effect);
				assert(casted_effect);
				action->addConstraint(new UnaryDomainBoundsConstraint(casted_effect, _problemInfo));
			} else if (arity == 2) {
				const auto* casted_effect = dynamic_cast<const BinaryScopedEffect *>(effect);
				assert(casted_effect);
				action->addConstraint(new BinaryDomainBoundsConstraint(casted_effect, _problemInfo));
			} else {
				std::stringstream buffer;
				buffer << "Error: Action " << action->getName();
				buffer << " has effects with arity > 2, which are currently unsupported" << std::endl;
				throw UnimplementedFeatureException(buffer.str());
			}

			++num_bconstraints;
		}
	}

	std::cout << "Added a total of " << num_bconstraints << " bound-constraints to the " <<  _actions.size() << " problem actions." << std::endl;
}
*/

/*
void Problem::compileConstraints() {
	unsigned num_compiled = 0;
	for (Action::ptr action:_actions) {
		num_compiled += compileConstraintVector(action->getConstraints());
	}
	num_compiled += compileConstraintVector(stateConstraints);
	num_compiled += compileConstraintVector(goalConstraints);

	std::cout << "Compiled a total of " << num_compiled << " constraints." << std::endl;
}

unsigned Problem::compileConstraintVector(ScopedConstraint::vcptr& constraints) const {
	unsigned num_compiled = 0;
	for (unsigned i = 0; i < constraints.size(); ++i) {
		ScopedConstraint::cptr compiled = constraints[i]->compile(_problemInfo);
		if (compiled) { // The constraint type requires pre-compilation
			delete constraints[i];
			constraints[i] = compiled;
			++num_compiled;
		}
	}
	return num_compiled;
}
*/

ApplicableActionSet Problem::getApplicableActions(const State& s) const {
	return ApplicableActionSet(ApplicabilityManager(getStateConstraints()), s, _ground);
}


std::ostream& Problem::print(std::ostream& os) const { 
	const fs0::ProblemInfo& info = getProblemInfo();
	os << "Planning Problem [domain:" << info.getDomainName() << ", instance: " << info.getInstanceName() <<  "]" << std::endl;
	
	os << "Action schemata" << std::endl;
	for (const ActionSchema::cptr elem:_schemata) {
		os << *elem << std::endl;
	}
	
	os << "Ground Actions" << std::endl;
	for (const GroundAction::cptr elem:_ground) {
		os << *elem << std::endl;
	}
	
	return os;
}

} // namespaces
