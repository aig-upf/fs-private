
#include <problem.hxx>
#include <constraints/scoped_constraint.hxx>
#include <actions.hxx>
#include <relaxed_action_manager.hxx>
#include <sstream>
#include <constraints/gecode_constraint_manager.hxx>
#include <constraints/gecode/translators.hxx>

#include <utils/logging.hxx>

namespace fs0 {

const Problem* Problem::_instance = 0;

Problem::Problem(const rapidjson::Document& data) :
	_problemInfo(data),
	stateConstraints(),
	goalConstraints()
{
}

Problem::~Problem() {
	for (const auto pointer:_actions) delete pointer;
	for (const auto pointer:stateConstraints) delete pointer;
	for (const auto pointer:goalConstraints) delete pointer;
	delete ctrManager;
}


void Problem::bootstrap() {
	// Safety check
	if (goalConstraints.empty()) {
		throw std::runtime_error(
			std::string("No goal specification detected. The problem is probably being bootstrapped before having been fully") + 
			 "initialized with the per-instance generate() procedure");
	}
	
	gecode::registerTranslators();
	
	// Add the necessary (unary) constraints to keep the appropriate values, if any, within their bounds.
	addDomainBoundConstraints();

	// Compile the constraints if necessary
	compileConstraints();

	// Create the constraint manager
	ctrManager = PlanningConstraintManagerFactory::create(goalConstraints, stateConstraints);
	FDEBUG("main", "Generated goal CSP:" << std::endl <<  *ctrManager << std::endl);
	

	// Creates the appropriate applicability manager depending on the type and arity of action precondition constraints.
	ActionManagerFactory::instantiate(*this, _actions);

	_goalRelevantVars.resize(_problemInfo.getNumVariables());
	for ( unsigned i = 0; i < _goalRelevantVars.size(); i++ )
		_goalRelevantVars[i] = true;
}

//! For every goal, precondition and global constraint, we note the name of
//! the variables in their scope inside the _relevantVars array
void
Problem::analyzeVariablesRelevance() {
	assert( !getGoalConstraints().empty() );
	for ( unsigned i = 0; i < _goalRelevantVars.size(); i++ )
		_goalRelevantVars[i] = false;

	for ( ScopedConstraint::cptr global : getGoalConstraints() )
		for ( auto x : global->getScope() ) _goalRelevantVars[x] = true;

	for ( ScopedConstraint::cptr global : getConstraints() )
		for ( auto x : global->getScope() ) _goalRelevantVars[x] = true;

	for ( auto a : getAllActions() ) {
		for ( ScopedConstraint::cptr prec : a->getConstraints() ) {
			UnaryDomainBoundsConstraint* conc_1 = dynamic_cast<UnaryDomainBoundsConstraint*>(prec);
			if (conc_1 != nullptr ) continue;
			BinaryDomainBoundsConstraint* conc_2 = dynamic_cast<BinaryDomainBoundsConstraint*>(prec);
			if (conc_2 != nullptr ) continue;

			for ( auto x : prec->getScope() ) _goalRelevantVars[x] = true;
		}
	}

	unsigned numGoalRelevantVars = 0;
	for ( unsigned i = 0; i < _goalRelevantVars.size(); i++ )
		numGoalRelevantVars = ( _goalRelevantVars[i] ? numGoalRelevantVars+1 : numGoalRelevantVars );
	std::cout << "# State variables relevant to the goal: " << numGoalRelevantVars << std::endl;

}

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

SimpleApplicableActionSet Problem::getApplicableActions(const State& s) const {
	return SimpleApplicableActionSet(StandardApplicabilityManager(s, getConstraints()), _actions);
}

} // namespaces
