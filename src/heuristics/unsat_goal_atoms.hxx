
#pragma once

#include <problem.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs0 {

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class UnsatisfiedGoalAtomsHeuristic {
public:
	UnsatisfiedGoalAtomsHeuristic(const Problem& problem) : _goal_conjunction(extract_goal_conjunction(problem)) {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const { 
		unsigned unsatisfied = 0;
		for (const fs::Formula* condition:get_goal_conjuncts()) {
			if (!condition->interpret(state)) ++unsatisfied;
		}
		return unsatisfied;
	}
	
	const std::vector<const fs::Formula*>& get_goal_conjuncts() const {
		return _goal_conjunction->getSubformulae();
	}
	
	
protected:
	const std::unique_ptr<const fs::Conjunction> _goal_conjunction;
	
	const fs::Conjunction* extract_goal_conjunction(const Problem& problem) {
		auto clone = problem.getGoalConditions()->clone();
		auto goal_conjunction = dynamic_cast<const fs::Conjunction*>(clone);
		if (!goal_conjunction) { // If the goal is now inside a conjunction, we wrap it up inside a new one.
			goal_conjunction = new fs::Conjunction({clone});
		}
		return goal_conjunction;
	}
};

} // namespaces
