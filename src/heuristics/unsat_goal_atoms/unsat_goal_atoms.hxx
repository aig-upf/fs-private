
#pragma once

#include <fs0_types.hxx>
#include <actions/ground_action.hxx>
#include <state_model.hxx>
#include <problem.hxx>

namespace fs0 {

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class UnsatisfiedGoalAtomsHeuristic {
public:
	typedef GroundAction Action;

	UnsatisfiedGoalAtomsHeuristic(const FS0StateModel& model) : _problem(model.getTask()) {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const { 
		unsigned unsatisfied = 0;
		const std::vector<AtomicFormula::cptr>& conditions = _problem.getGoalConditions();
		for (AtomicFormula::cptr condition:conditions) {
			if (condition->interpret(state)) {
				++unsatisfied;
			}
		}
		return unsatisfied;
	}
	
protected:
	//! The actual planning problem
	const Problem& _problem;
};

} // namespaces
