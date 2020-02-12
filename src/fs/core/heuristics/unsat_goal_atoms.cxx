

#include <fs/core/problem.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>

#include <fs/core/heuristics/unsat_goal_atoms.hxx>
#include <fs/core/languages/fstrips/complex_existential_formula.hxx>

namespace fs0 {

UnsatisfiedGoalAtomsCounter::UnsatisfiedGoalAtomsCounter(const fs::Formula* formula, const AtomIndex& atomidx) :
	_formula_atoms(extract_formula_components(formula, atomidx))
{}

unsigned UnsatisfiedGoalAtomsCounter::evaluate(const State& state) const {
    unsigned unsatisfied = 0;
	for (const auto& condition:_formula_atoms) {
		if (!condition->interpret(state)) ++unsatisfied;
	}
	return unsatisfied;
}


std::vector<std::shared_ptr<const fs::Formula>>
extract_formula_components(const fs::Formula* formula, const AtomIndex& atomidx) {
	std::vector<std::shared_ptr<const fs::Formula>> atoms;

	const auto* conjunction = dynamic_cast<const fs::Conjunction*>(formula);
	const auto* ex_q = dynamic_cast<const fs::ExistentiallyQuantifiedFormula*>(formula);

	if (conjunction) { // Wrap out the conjuncts
		for (const auto& a:conjunction->getSubformulae()) {
			atoms.push_back(std::shared_ptr<const fs::Formula>(a->clone()));
		}
	} else if (ex_q) { // If we have an existentially-quantified formula, we'll use
                       // Gecode to evaluate it
		atoms.push_back(std::shared_ptr<const fs::Formula>(new fs::ComplexExistentialFormula(ex_q, atomidx)));
	} else {
		atoms.push_back(std::shared_ptr<const fs::Formula>(formula->clone()));
	}

	return atoms;
}

} // namespaces
