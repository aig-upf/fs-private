

#include <problem.hxx>
#include <state.hxx>
#include <heuristics/unsat_goal_atoms.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/operations/basic.hxx>
#include <lapkt/tools/logging.hxx>
#include <deque>

namespace fs0 {

UnsatisfiedGoalAtomsHeuristic::UnsatisfiedGoalAtomsHeuristic(const Problem& problem) : _goal_conjunction(extract_goal_conjunction(problem)) {}

const fs::RelationalFormula*
clone( const fs::RelationalFormula* o ) {
    std::vector< const fs::Term* > st;
    for ( auto t : o->getSubterms() )
        st.push_back(t->clone());
    return o->clone(st);
}

float UnsatisfiedGoalAtomsHeuristic::evaluate(const State& state) const {
	unsigned unsatisfied = 0;
    LPT_DEBUG("heuristic", "Pushing boundaries out...");
    LPT_DEBUG("heuristic", "State:\n" << state );

	for (const fs::Formula* condition:get_goal_conjuncts()) {

		//if (!condition->interpret(state)) ++unsatisfied;

        if ( condition->interpret(state) ) continue;

        std::vector< const fs::RelationalFormula* > relationals = fs::all_relations(*condition);

        if (relationals.empty()) {
            unsatisfied++;
            continue;
        }

        std::deque< const fs::RelationalFormula* > boundary;
        for ( auto f: relationals )
            boundary.push_back( clone(f) ); // MRJ: we retain ownership
        while (!boundary.empty()) {
            const fs::RelationalFormula* poly = boundary.front();
            LPT_DEBUG("heuristic", "\tEvaluating:" << *poly );
            boundary.pop_front();
            if (poly->interpret(state)) {
                delete poly;
                continue;
            }
            unsatisfied++;
            //MRJ: Magic constant!
            std::vector<fs::RelationalFormula*> Rpoly = poly->relax(fs::NumericConstant(0.1f));
            for ( auto f : Rpoly )
                boundary.push_back(f);
            delete poly;
        }

	}
    LPT_DEBUG("heuristic", "h=" << unsatisfied );
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
