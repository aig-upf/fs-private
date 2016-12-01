
#pragma once

#include <fs_types.hxx>
#include <actions/actions.hxx>
#include <ground_state_model.hxx>
#include <problem.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs0 {

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
template <typename StateModelT>
class UnsatisfiedGoalAtomsHeuristic {
public:
	typedef GroundAction Action;

	UnsatisfiedGoalAtomsHeuristic(const StateModelT& model) : _problem(model.getTask()), _goal_conjunction(extract_goal_conjunction_or_fail(_problem)) {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const { 
		unsigned unsatisfied = 0;
		for (const fs::AtomicFormula* condition:get_goal_conjuncts()) {
			if (!condition->interpret(state)) ++unsatisfied;
		}
		return unsatisfied;
	}
	
	const std::vector<const fs::AtomicFormula*>& get_goal_conjuncts() const {
		return _goal_conjunction->getConjuncts();
	}
	
	
protected:
	//! The actual planning problem
	const Problem& _problem;
	
	const fs::Conjunction* _goal_conjunction;
	
	const fs::Conjunction* extract_goal_conjunction_or_fail(const Problem& problem) {
		auto goal_conjunction = dynamic_cast<const fs::Conjunction*>(problem.getGoalConditions());
		if (!goal_conjunction) throw std::runtime_error("UnsatisfiedGoalAtomsHeuristic valid only if the goal is a conjunction of atoms");
		return goal_conjunction;
	}
};

} // namespaces
