
#pragma once

#include <languages/fstrips/language_fwd.hxx>

namespace fs0 {

class Problem;

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class L0Heuristic {
public:
	L0Heuristic(const Problem& problem);
    ~L0Heuristic();

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const;

	const std::vector<const fs::Formula*>& get_goal_conjuncts() const;

    const std::vector<const fs::Formula*>& non_relational() const { return _non_relational_goals; }
    const std::vector<const fs::Formula*>& relational() const { return _goal_oriented_hyperplanes; }

protected:
	const std::unique_ptr<const fs::Conjunction> _goal_conjunction;

	const fs::Conjunction* extract_goal_conjunction(const Problem& problem);

    std::vector< const fs::Formula* >       _non_relational_goals;
    std::vector< const fs::Formula* >       _goal_oriented_hyperplanes;
};

} // namespaces
