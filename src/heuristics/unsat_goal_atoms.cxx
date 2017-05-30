

#include <problem.hxx>
#include <state.hxx>
#include <heuristics/unsat_goal_atoms.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/operations/basic.hxx>
#include <lapkt/tools/logging.hxx>
#include <deque>

namespace fs0 {

UnsatisfiedGoalAtomsHeuristic::UnsatisfiedGoalAtomsHeuristic(const Problem& problem)
    : _goal_conjunction(extract_goal_conjunction(problem)) {
}

UnsatisfiedGoalAtomsHeuristic::~UnsatisfiedGoalAtomsHeuristic() {
}


float UnsatisfiedGoalAtomsHeuristic::evaluate(const State& state) const {
	unsigned unsatisfied = 0;
    for ( auto f : get_goal_conjuncts() )
        if ( !f->interpret(state) )
            unsatisfied++;
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
