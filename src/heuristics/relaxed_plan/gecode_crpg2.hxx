
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/extensions.hxx>
#include <atom.hxx>
#include <utils/index.hxx>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace gecode {

class BaseActionCSPHandler;
class GecodeRPGBuilder;
class GecodeRPGLayer;

class ConstrainedRPG {
protected:
	typedef std::shared_ptr<BaseActionCSPHandler> ActionHandlerPtr;
	
public:
	ConstrainedRPG(const Problem& problem, std::vector<ActionHandlerPtr>&& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	
	virtual ~ConstrainedRPG() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const State& seed, const GecodeRPGLayer& layer, const RPGData& rpg);
	
protected:
	typedef std::vector<std::vector<ActionHandlerPtr>> AchieverIndex;
	
	
	//! The actual planning problem
	const Problem& _problem;
	
	//! The set of action managers, one per every action
	const std::vector<ActionHandlerPtr> _managers;
	
	//! The RPG building helper
	const std::shared_ptr<GecodeRPGBuilder> _builder;
	
	//!
	ExtensionHandler _extension_handler;
	
	//! An index of all the problem atoms.
	Index<Atom> _atom_idx;
	
	//! a map from atom index to the set of action / effect managers that can (potentially) achieve that atom.
	const AchieverIndex _atom_achievers;
	
	//! A helper to index all of the problem's atoms.
	static Index<Atom> index_atoms(const ProblemInfo& info);
	
	//! A helper to build the index of atom achievers.
	static AchieverIndex build_achievers_index(const std::vector<ActionHandlerPtr>& managers, const Index<Atom>& atom_idx);
};

} } // namespaces
