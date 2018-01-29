
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/constraints/gecode/extensions.hxx>
#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/constraints/gecode/handlers/lifted_effect_csp.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <unordered_set>
#include <fs/core/constraints/gecode/handlers/ground_effect_csp.hxx>
#include <fs/core/constraints/native/action_handler.hxx>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class RPGIndex;

class NativeRPG {
protected:
    using HandlerT = gecode::NativeActionHandler;
	using HandlerPT = std::unique_ptr<HandlerT>;

public:
	NativeRPG(const Problem& problem, const fs::Formula* goal_formula, const std::vector<const fs::Formula*>& state_constraints, std::vector<HandlerPT>&& managers, ExtensionHandler extension_handler);
	~NativeRPG() = default;

	// Disallow copies of the object, as they will be expensive, but allow moves.
	NativeRPG(const NativeRPG&) = delete;
	NativeRPG(NativeRPG&&) = default;
	NativeRPG& operator=(const NativeRPG& other) = delete;
	NativeRPG& operator=(NativeRPG&& other) = default;

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed, std::vector<Atom>& relevant);
	long evaluate(const State& seed) {
		std::vector<Atom> _; // Ignore the relevant values if not requested
		return evaluate(seed, _);
	}

	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const RPGIndex& graph, std::vector<Atom>& relevant);


protected:
	//! The actual planning problem
	const Problem& _problem;
	const ProblemInfo& _info;

	const AtomIndex& _tuple_index;

	//! The set of action managers, one per every action
	std::vector<HandlerPT> _managers;

	//!
	ExtensionHandler _extension_handler;

    SimpleFormulaChecker _goal_checker;
};

} } // namespaces
