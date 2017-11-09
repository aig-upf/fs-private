
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/constraints//gecode/extensions.hxx>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace gecode {

class BaseActionCSP;
class FormulaCSP;
class RPGIndex;

class GecodeCRPG {
public:
	GecodeCRPG(const Problem& problem, const fs::Formula* goal_formula, const std::vector<const fs::Formula*>& state_constraints, std::vector<std::shared_ptr<BaseActionCSP>>&& managers, ExtensionHandler extension_handler);
	virtual ~GecodeCRPG() = default;

	// Disallow copies of the object, as they will be expensive, but allow moves.
	GecodeCRPG(const GecodeCRPG&) = delete;
	GecodeCRPG(GecodeCRPG&&) = default;
	GecodeCRPG& operator=(const GecodeCRPG& other) = delete;
	GecodeCRPG& operator=(GecodeCRPG&& other) = default;

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed, std::vector<Atom>& relevant);
	long evaluate(const State& seed) {
		std::vector<Atom> _; // Ignore the relevant values if not requested
		return evaluate(seed, _);
	}

	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const RPGIndex& graph) const;

protected:
	//! The actual planning problem
	const Problem& _problem;

	const AtomIndex& _tuple_index;

	//! The set of action managers, one per every action
	const std::vector<std::shared_ptr<BaseActionCSP>> _managers;

	//!
	ExtensionHandler _extension_handler;

	std::unique_ptr<FormulaCSP> _goal_handler;
};

//! The h_max version
class GecodeCHMax : public GecodeCRPG {
public:
	GecodeCHMax(const Problem& problem, const fs::Formula* goal_formula, const std::vector<const fs::Formula*>& state_constraints, std::vector<std::shared_ptr<BaseActionCSP>>&& managers, ExtensionHandler extension_handler);
	~GecodeCHMax() = default;

	// Disallow copies of the object, as they will be expensive, but allow moves.
	GecodeCHMax(const GecodeCHMax&) = delete;
	GecodeCHMax(GecodeCHMax&&) = default;
	GecodeCHMax& operator=(const GecodeCHMax& other) = delete;
	GecodeCHMax& operator=(GecodeCHMax&& other) = default;

	//! The hmax heuristic only cares about the size of the RP graph.
	long computeHeuristic(const RPGIndex& graph) const override;
};

} } // namespaces
