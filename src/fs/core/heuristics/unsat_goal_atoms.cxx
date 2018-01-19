

#include <fs/core/problem.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>

#include <fs/core/heuristics/unsat_goal_atoms.hxx>
#include <fs/core/languages/fstrips/complex_existential_formula.hxx>

namespace fs0 {

UnsatisfiedGoalAtomsCounter::UnsatisfiedGoalAtomsCounter(const Problem& problem) :
	_formula_atoms(extract_goal_conjunction(problem))
{}

float UnsatisfiedGoalAtomsCounter::evaluate(const State& state) const {
	unsigned unsatisfied = 0;
	for (const auto& condition:_formula_atoms) {
		if (!condition->interpret(state)) ++unsatisfied;
	}
	return unsatisfied;
}


std::vector<const fs::Formula*>
UnsatisfiedGoalAtomsCounter::extract_goal_conjunction(const Problem& problem) {
	std::vector<const fs::Formula*> atoms;

	auto goal = problem.getGoalConditions();
	const auto* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	const auto* ex_q = dynamic_cast<const fs::ExistentiallyQuantifiedFormula*>(goal);

	if (conjunction) { // Wrap out the conjuncts
		for (const auto& a:conjunction->getSubformulae()) {
			atoms.push_back(a->clone());
		}
	} else if (ex_q) { // If we have an existentially-quantified formula, we'll use
                       // Gecode to evaluate it
		atoms.push_back(new fs::ComplexExistentialFormula(ex_q, problem.get_tuple_index()));
	} else {
		atoms.push_back(goal->clone());
	}

	return atoms;
}

} // namespaces
