
#include <applicability/applicability_manager.hxx>
#include <actions/ground_action.hxx>
#include <state.hxx>
#include <problem.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {


ApplicabilityManager::ApplicabilityManager(const fs::Formula::cptr state_constraints)
	: _state_constraints(state_constraints) {}
	
//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
bool ApplicabilityManager::isApplicable(const State& state, const GroundAction& action) const {
	if (!checkFormulaHolds(action.getPrecondition(), state)) return false;
	
	auto atoms = computeEffects(state, action);
	if (!checkAtomsWithinBounds(atoms)) return false;
		
	if (!_state_constraints->is_tautology()) { // If we have no constraints, we can spare the cost of creating the new state.
		State next(state, atoms);
		return checkFormulaHolds(_state_constraints, next);
	}
	return true;
}

//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
Atom::vctr ApplicabilityManager::computeEffects(const State& state, const GroundAction& action) const {
	Atom::vctr atoms;
	for (const fs::ActionEffect::cptr effect:action.getEffects()) {
		atoms.push_back(effect->apply(state));
	}
	return atoms;
}

bool ApplicabilityManager::checkFormulaHolds(const fs::Formula::cptr formula, const State& state) {
	return formula->interpret(state);
}

bool ApplicabilityManager::checkAtomsWithinBounds(const std::vector<Atom>& atoms) {
	const ProblemInfo& info = Problem::getInfo();
	for (const auto& atom:atoms) {
		if (!info.checkValueIsValid(atom)) return false;
	}
	return true;
}

} // namespaces
