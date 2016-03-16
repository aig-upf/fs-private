
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/extensions.hxx>
#include <atom.hxx>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace gecode {

class BaseActionCSPHandler;
class GecodeRPGBuilder;
class GecodeRPGLayer;

class ConstrainedRPG {
public:
	ConstrainedRPG(const Problem& problem, std::vector<std::shared_ptr<BaseActionCSPHandler>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	
	virtual ~ConstrainedRPG() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const State& seed, const GecodeRPGLayer& layer, const RPGData& rpg);
	
protected:
	//! The actual planning problem
	const Problem& _problem;
	
	//! The set of action managers, one per every action
	const std::vector<std::shared_ptr<BaseActionCSPHandler>> _managers;
	
	//! The RPG building helper
	const std::shared_ptr<GecodeRPGBuilder> _builder;
	
	//!
	ExtensionHandler _extension_handler;
	
	//! An index of all the problem atoms.
	std::vector<Atom> _atom_idx;
	
	//! A helper to index all of the problem's atoms.
	static std::vector<Atom> build_atom_index(const ProblemInfo& info);
};

} } // namespaces
