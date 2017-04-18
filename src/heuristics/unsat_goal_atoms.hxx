
#pragma once

#include <languages/fstrips/language_fwd.hxx>

namespace fs0 {

class Problem;

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class UnsatisfiedGoalAtomsHeuristic {
public:
	UnsatisfiedGoalAtomsHeuristic(const Problem& problem);
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& state) const;
	
	const std::vector<const fs::Formula*>& get_goal_conjuncts() const;
	
	
protected:
	const std::unique_ptr<const fs::Conjunction> _goal_conjunction;
	
	const fs::Conjunction* extract_goal_conjunction(const Problem& problem);
};

} // namespaces
