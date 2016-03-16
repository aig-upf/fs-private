
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/extensions.hxx>
#include <atom.hxx>
#include <utils/index.hxx>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace gecode {

class BaseActionCSPHandler;
class GroundEffectCSPHandler;
class GecodeRPGBuilder;
class GecodeRPGLayer;

class ConstrainedRPG {
protected:
	typedef std::shared_ptr<BaseActionCSPHandler> ActionHandlerPtr;
	typedef std::shared_ptr<GroundEffectCSPHandler> EffectHandlerPtr;
	
public:
	ConstrainedRPG(const Problem& problem, const std::vector<ActionHandlerPtr>& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	
	virtual ~ConstrainedRPG() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const State& seed, const GecodeRPGLayer& layer, const RPGData& rpg);
	
protected:
	typedef std::vector<std::vector<unsigned>> AchieverIndex;
	
	
	//! The actual planning problem
	const Problem& _problem;
	
	//! The set of action managers, one per every action
	const std::vector<EffectHandlerPtr> _managers;
	
	//! The RPG building helper
	const std::shared_ptr<GecodeRPGBuilder> _builder;
	
	//!
	ExtensionHandler _extension_handler;
	
	//! An index of all the problem atoms.
	Index<Atom> _atom_table;
	
	//! a map from atom index to the set of action / effect managers that can (potentially) achieve that atom.
	//! let L = _atom_achievers[i] be the vector of all potential achievers of atom with index 'i'.
	//! Then each element j in L is the index of an effect manager in '_managers'.
	const AchieverIndex _atom_achievers;
	
	//! If atom with index 'i' has state variable f(t_1, ..., t_n), then '_atom_variable_tuples[i]' is the vector {t_1, ..., t_n}
	const std::vector<std::vector<ObjectIdx>> _atom_variable_tuples;
	
	static std::vector<std::vector<ObjectIdx>> index_variable_tuples(const ProblemInfo& info, const Index<Atom>& index);
	
	//! A helper to index all of the problem's atoms.
	static Index<Atom> index_atoms(const ProblemInfo& info);
	
	//! A helper to build the index of atom achievers.
	static AchieverIndex build_achievers_index(const std::vector<EffectHandlerPtr>& managers, const Index<Atom>& atom_idx);
	
	//! Check that all the given managers are indeed effect managers, and downcast them
	static std::vector<EffectHandlerPtr> downcast_managers(const std::vector<ActionHandlerPtr>& managers);
};

} } // namespaces
