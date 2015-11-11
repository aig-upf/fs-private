
#pragma once

#include <fs0_types.hxx>
#include <actions/ground_action.hxx>
#include <state_model.hxx>
#include <problem.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs0 {

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class UnsatisfiedGoalAtomsHeuristic {
public:
	typedef GroundAction Action;

	UnsatisfiedGoalAtomsHeuristic(const FS0StateModel& model) : _problem(model.getTask()), _goal_conjunction(extract_goal_conjunction_or_fail(_problem)) {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const { 
		unsigned unsatisfied = 0;
		for (AtomicFormula::cptr condition:_goal_conjunction->getConjuncts()) {
			if (!condition->interpret(state)) ++unsatisfied;
		}
		return unsatisfied;
	}
	
protected:
	//! The actual planning problem
	const Problem& _problem;
	
	Conjunction::cptr _goal_conjunction;
	
	Conjunction::cptr extract_goal_conjunction_or_fail(const Problem& problem) {
		auto goal_conjunction = dynamic_cast<Conjunction::cptr>(problem.getGoalConditions());
		if (goal_conjunction) throw std::runtime_error("UnsatisfiedGoalAtomsHeuristic valid only if the goal is a conjunction of atoms");
		return goal_conjunction;
	}
};

} // namespaces
