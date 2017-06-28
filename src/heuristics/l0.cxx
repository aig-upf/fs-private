
#include <problem.hxx>
#include <state.hxx>
#include <heuristics/l0.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/operations/basic.hxx>
#include <lapkt/tools/logging.hxx>
#include <deque>

namespace fs0 {

namespace detail {
    const fs::RelationalFormula*
    clone( const fs::RelationalFormula* o ) {
        std::vector< const fs::Term* > st;
        for ( auto t : o->getSubterms() )
            st.push_back(t->clone());
        return o->clone(st);
    }
}

L0Heuristic::L0Heuristic(const Problem& problem)
    : _goal_conjunction(extract_goal_conjunction(problem)) {

    const State& state = problem.getInitialState();
    // MRJ: we will build here our polytopes
    LPT_DEBUG("heuristic", "Pushing boundaries out...");
    LPT_DEBUG("heuristic", "State:\n" << state );

	for (const fs::Formula* condition:get_goal_conjuncts()) {
        std::vector< const fs::RelationalFormula* > relationals = fs::all_relations(*condition);

        if (relationals.empty()) {
            _non_relational_goals.push_back( condition );
            continue;
        }

        std::deque< const fs::RelationalFormula* > boundary;
        for ( auto f: relationals )
            boundary.push_back( detail::clone(f) ); // MRJ: we retain ownership
        while (!boundary.empty()) {
            const fs::RelationalFormula* poly = boundary.front();
            boundary.pop_front();
            LPT_DEBUG("heuristic", "Added new hyperplane: " << *poly );
            _goal_oriented_hyperplanes.push_back(poly);
            //MRJ: Magic constant!
            if ( poly->interpret(state) ) {
                continue;
            }
            std::vector<fs::RelationalFormula*> Rpoly = poly->relax(fs::NumericConstant(0.1f));
            for ( auto f : Rpoly )
                boundary.push_back(f);
        }

	}

}

L0Heuristic::~L0Heuristic() {
    for ( auto f : _goal_oriented_hyperplanes )
        delete f;
}


float L0Heuristic::evaluate(const State& state) const {
	unsigned unsatisfied = 0;

    unsigned relational = 0;
    unsigned non_relational = 0;
    std::vector<const fs::Formula* > unsat;

    for ( const fs::Formula* condition : _non_relational_goals ) {
        if ( condition->interpret(state) ) {
            unsat.push_back(condition);
            continue;
        }

        non_relational++;
        unsatisfied++;

    }

    for (const fs::Formula* condition : _goal_oriented_hyperplanes ) {
        if ( condition->interpret(state) ) {
            unsat.push_back(condition);
            continue;
        }
        relational++;
        unsatisfied++;
    }

    LPT_DEBUG("heuristic", "h=" << unsatisfied << ",relational=" << relational << ",non-relational=" << non_relational );
    if ( unsat.size() == 1 ) {
        LPT_DEBUG("heuristic", "State:\n" << state );
        LPT_DEBUG("heuristic", "Lone sad goal: " << *unsat[0]);
    }
	return unsatisfied;
}

const std::vector<const fs::Formula*>&
L0Heuristic::get_goal_conjuncts() const {
	return _goal_conjunction->getSubformulae();
}



const fs::Conjunction*
L0Heuristic::extract_goal_conjunction(const Problem& problem) {
	auto clone = problem.getGoalConditions()->clone();
	auto goal_conjunction = dynamic_cast<const fs::Conjunction*>(clone);
	if (!goal_conjunction) { // If the goal is now inside a conjunction, we wrap it up inside a new one.
		goal_conjunction = new fs::Conjunction({clone});
	}
	return goal_conjunction;
}

} // namespaces
