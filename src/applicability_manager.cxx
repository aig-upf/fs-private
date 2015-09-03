
#include <applicability_manager.hxx>
#include <actions/ground_action.hxx>
#include <state.hxx>
#include <problem.hxx>


namespace fs0 {


ApplicabilityManager::ApplicabilityManager(const std::vector<AtomicFormula::cptr>& state_constraints)
	: _state_constraints(state_constraints) {}
	
//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
bool ApplicabilityManager::isApplicable(const State& state, const GroundAction& action) const {
	if (!checkFormulaHolds(action.getConditions(), state)) return false;
	
	auto atoms = computeEffects(state, action);
	if (!checkAtomsWithinBounds(atoms)) return false;
		
	if (_state_constraints.size() != 0) { // If we have no constraints, we can spare the cost of creating the new state.
		State next(state, atoms);
		return checkFormulaHolds(_state_constraints, next);
	}
	return true;
}

//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
Atom::vctr ApplicabilityManager::computeEffects(const State& state, const GroundAction& action) const {
	Atom::vctr atoms;
	for (const ActionEffect::cptr effect:action.getEffects()) {
		atoms.push_back(effect->apply(state));
	}
	return atoms;
}

bool ApplicabilityManager::checkFormulaHolds(const std::vector<AtomicFormula::cptr>& formula, const State& state) {
	for (AtomicFormula::cptr atomic:formula) {
		if (!atomic->interpret(state)) return false;
	}
	return true;
}

bool ApplicabilityManager::checkAtomsWithinBounds(const std::vector<Atom>& atoms) {
	const ProblemInfo& info = Problem::getInfo();
	for (const auto& atom:atoms) {
		if (!info.checkValueIsValid(atom)) return false;
	}
	return true;
}

} // namespaces
