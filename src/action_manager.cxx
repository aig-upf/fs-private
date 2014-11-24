

#include <action_manager.hxx>
#include <core_problem.hxx>
#include <actions.hxx>
#include <utils/cartesian_product.hxx>


namespace aptk { namespace core {


// State::ptr ActionManager::applyAction(const CoreAction::cptr& action, const State::ptr& s0) {
// 	SimpleActionSetManager manager(*s0);
// 	JustifiedAction justified(*action);
// 	
// 	if (!manager.isApplicable(justified)) {
// 		return RelaxedState::ptr();
// 	}
// 	
// 	Changeset changeset;
// 	manager.computeChangeset(justified, changeset);
// 	return std::make_shared<RelaxedState>(*s0, changeset); // Copy everything into the new state and apply the changeset
// }


// template <class S>
// bool ActionManager::checkFormulaSatisfiable(const PlainConjunctiveFact& formula, const S& state) {
// 	for(auto fact:formula.elements) {
// 		if (!fact->isSatisfiable(state)) {
// 			return false;
// 		}
// 	}
// 	return true;
// }
// 
// 
// template <class S>
// bool ActionManager::checkFormulaSatisfiable(const Fact& fact, const S& state) { return state.contains(fact); }
// 
// 
// bool ActionManager::checkFormulaSatisfiable(const NegatedFact& fact, const State& state) { return !state.contains(fact); }
// 
// bool ActionManager::checkFormulaSatisfiable(const NegatedFact& fact, const RelaxedState& state) {
// 	const ObjectSet& possibleValues = state.getValues(fact._variable);
// 	// Either we have more than one possible value, or the one that we have is NOT the one in the fact.
// 	return possibleValues.size() > 1  || possibleValues.find(fact._value) == possibleValues.end();
// }

} } // namespaces
