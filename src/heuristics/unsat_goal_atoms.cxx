

#include <problem.hxx>
#include <heuristics/unsat_goal_atoms.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs0 {

UnsatisfiedGoalAtomsHeuristic::UnsatisfiedGoalAtomsHeuristic(const Problem& problem)
	: _goal_conjunction(extract_goal_conjunction(problem)) {}
	
float UnsatisfiedGoalAtomsHeuristic::evaluate(const State& state) const {
	unsigned unsatisfied = 0;
	for (const fs::Formula* condition:get_goal_conjuncts()) {
		if (!condition->interpret(state)) ++unsatisfied;
	}
	return unsatisfied;
}

const std::vector<const fs::Formula*>&
UnsatisfiedGoalAtomsHeuristic::get_goal_conjuncts() const {
	return _goal_conjunction->getSubformulae();
}



const fs::Conjunction*
UnsatisfiedGoalAtomsHeuristic::extract_goal_conjunction(const Problem& problem) {
	auto clone = problem.getGoalConditions()->clone();
	auto goal_conjunction = dynamic_cast<const fs::Conjunction*>(clone);
	if (!goal_conjunction) { // If the goal is now inside a conjunction, we wrap it up inside a new one.
		goal_conjunction = new fs::Conjunction({clone});
	}
	return goal_conjunction;
}

} // namespaces
