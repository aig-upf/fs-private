
#pragma once

#include <fs/core/languages/fstrips/language_fwd.hxx>
#include <vector>
#include <memory>

namespace fs0 {

class Problem;

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class UnsatisfiedGoalAtomsCounter {
public:
	UnsatisfiedGoalAtomsCounter(const Problem& problem);

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const;

protected:
	const std::vector<const fs::Formula*> _formula_atoms;

	std::vector<const fs::Formula*> extract_goal_conjunction(const Problem& problem);

};

} // namespaces
