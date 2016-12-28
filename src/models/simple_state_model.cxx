
#include "simple_state_model.hxx"
#include <problem.hxx>
#include <state.hxx>
#include <applicability/formula_interpreter.hxx>
#include <utils/config.hxx>
#include <applicability/match_tree.hxx>
#include <aptk2/tools/logging.hxx>

#include <languages/fstrips/language.hxx>

// #include <lapkt/multivalued_state.hxx>

namespace fs0 {


//! A helper to derive the distinct goal atoms
/*
const std::vector<const fs::AtomicFormula*>&
obtain_goal_atoms(const fs::Formula* goal) {
	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	if (!conjunction) {
		throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
	}
	return conjunction->getConjuncts();
}
*/


//! A helper to derive the distinct goal atoms
std::vector<Atom>
obtain_goal_atoms(const fs::Formula* goal) {
	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	if (!conjunction) {
		throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
	}
	
	std::vector<Atom> goal_atoms;
	
	for (const fs::AtomicFormula* atom:conjunction->getConjuncts()) {
		auto eq =  dynamic_cast<const fs::EQAtomicFormula*>(atom);
		if (!eq) { // This could be easily extended to negated atoms
			throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
		}
		
		auto sv =  dynamic_cast<const fs::StateVariable*>(eq->lhs());
		if (!sv) throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
		
		auto ct =  dynamic_cast<const fs::Constant*>(eq->rhs());
		if (!ct) throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
		
		goal_atoms.push_back(Atom(sv->getValue(), ct->getValue()));
	}
	
	return goal_atoms;
}


SimpleStateModel
SimpleStateModel::build(const Problem& problem, BasicApplicabilityAnalyzer* analyzer) {
	return SimpleStateModel(problem, obtain_goal_atoms(problem.getGoalConditions()), analyzer);
}

SimpleStateModel::SimpleStateModel(const Problem& problem, std::vector<Atom> subgoals, BasicApplicabilityAnalyzer* analyzer) :
	_task(problem),
	_manager(build_action_manager(problem, analyzer)),
	_subgoals(subgoals)
{}

SimpleStateModel::StateT
SimpleStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// This is only called once per search.
	return StateT(_task.getInitialState());
}

bool
SimpleStateModel::goal(const StateT& state) const {
	return _task.getGoalSatManager().satisfied(state);
}

bool
SimpleStateModel::is_applicable(const StateT& state, const ActionId& action) const {
	return is_applicable(state, *(_task.getGroundActions()[action]));
}

bool
SimpleStateModel::is_applicable(const StateT& state, const ActionType& action) const {
	return _manager->applicable(state, action);
}

SimpleStateModel::StateT
SimpleStateModel::next(const StateT& state, const GroundAction::IdType& actionIdx) const {
	return next(state, *(_task.getGroundActions()[actionIdx]));
}

SimpleStateModel::StateT
SimpleStateModel::next(const StateT& state, const GroundAction& a) const {
	NaiveApplicabilityManager::computeEffects(state, a, _effects_cache);
	return StateT(state, _effects_cache); // Copy everything into the new state and apply the changeset
}

bool
SimpleStateModel::goal(const StateT& s, unsigned i) const {
	return s.contains(_subgoals.at(i)); // TODO SHOULD BE:
	// const Atom& subgoal = _subgoals.at(i);
	// return s.check(subgoal.getVariable(), s.getValue());
}


GroundApplicableSet
SimpleStateModel::applicable_actions(const StateT& state) const {
	return _manager->applicable(state);
}

SmartActionManager*
SimpleStateModel::build_action_manager(const Problem& problem, BasicApplicabilityAnalyzer* analyzer) {
	const auto& actions = problem.getGroundActions();
	const auto& constraints = problem.getStateConstraints();
	const auto& tuple_idx =  problem.get_tuple_index();
	if (analyzer == nullptr) {
		analyzer = new BasicApplicabilityAnalyzer(actions, tuple_idx);
		analyzer->build();
	}
	if ( Config::instance().getSuccessorGeneratorType() == Config::SuccessorGenerationStrategy::functional_aware) {
		LPT_INFO( "main", "Successor Generator Strategy: \"Functional Aware\"");
		return new SmartActionManager(actions, constraints, tuple_idx, analyzer);
	}
	LPT_INFO( "main", "Successor Generator Strategy: \"Match Tree\"");
	return new MatchTreeActionManager( actions, constraints, tuple_idx, analyzer );
}


} // namespaces
