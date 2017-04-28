
#pragma once

#include <fs_types.hxx>
#include <constraints/gecode/extensions.hxx>
#include <constraints/gecode/handlers/formula_csp.hxx>
#include <constraints/gecode/handlers/lifted_effect_unreached.hxx>


namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace gecode {

class UnreachedAtomRPG {
public:
	using HandlerT = LiftedEffectUnreachedCSP;
	using HandlerPT = std::unique_ptr<HandlerT>;

	UnreachedAtomRPG(   const Problem& problem,
                        const fs::Formula* goal_formula,
                        const std::vector<const fs::Formula*>& state_constraints,
                        std::vector<HandlerPT>&& managers,
                        ExtensionHandler extension_handler);
	~UnreachedAtomRPG() = default;

	// Disallow copies of the object, as they will be expensive, but allow moves.
	UnreachedAtomRPG(const UnreachedAtomRPG&) = delete;
	UnreachedAtomRPG(UnreachedAtomRPG&&) = default;
	UnreachedAtomRPG& operator=(const UnreachedAtomRPG& other) = delete;
	UnreachedAtomRPG& operator=(UnreachedAtomRPG&& other) = default;

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed, std::vector<Atom>& relevant);
	long evaluate(const State& seed) {
		std::vector<Atom> _; // Ignore the relevant values if not requested
		return evaluate(seed, _);
	}

	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const RPGIndex& graph);

protected:

	//! The actual planning problem
	const Problem& _problem;

	const AtomIndex& _tuple_index;

	//! The set of action managers, one per every action
	std::vector<HandlerPT> _managers;

	std::unique_ptr<FormulaCSP> _goal_handler;

	//!
	ExtensionHandler _extension_handler;


	//! a map from atom index to the set of action / effect managers that can (potentially) achieve that atom.
	//! let L = _atom_achievers[i] be the vector of all potential achievers of atom with index 'i'.
	//! Then each element j in L is the index of an effect manager in '_managers'.
	typedef std::vector<std::vector<unsigned>> AchieverIndex;
	AchieverIndex _atom_achievers;

	//! A helper to build the index of atom achievers.
	static AchieverIndex build_achievers_index(const std::vector<HandlerPT>& managers, const AtomIndex& tuple_index);
};

} } // namespaces
