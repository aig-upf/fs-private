
#pragma once

#include <fs/core/languages/fstrips/language_fwd.hxx>
#include <vector>
#include <memory>

namespace fs0 {

class Problem;

std::vector<std::shared_ptr<const fs::Formula>> extract_formula_components(const fs::Formula* formula, const AtomIndex&);

//! The heuristic value of any given state is the number of unsatisfied goal conditions (atoms) on that state
class UnsatisfiedGoalAtomsCounter {
public:
	explicit UnsatisfiedGoalAtomsCounter(const fs::Formula* formula, const AtomIndex&);
	~UnsatisfiedGoalAtomsCounter() = default;

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	unsigned evaluate(const State& state) const;

protected:
	const std::vector<std::shared_ptr<const fs::Formula>> _formula_atoms;
};

} // namespaces
