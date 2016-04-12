
#pragma once

#include <fs_types.hxx>
#include <constraints/gecode/extensions.hxx>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace gecode {

class BaseActionCSPHandler;
class GecodeRPGBuilder;
class GecodeRPGLayer;

class GecodeCRPG {
public:
	GecodeCRPG(const Problem& problem, std::vector<std::shared_ptr<BaseActionCSPHandler>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	virtual ~GecodeCRPG() = default;
	
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
};

//! The h_max version
class GecodeCHMax : public GecodeCRPG {
public:
	GecodeCHMax(const Problem& problem, std::vector<std::shared_ptr<BaseActionCSPHandler>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	~GecodeCHMax() = default;
	
	//! The hmax heuristic only cares about the size of the RP graph.
	long computeHeuristic(const State& seed, const GecodeRPGLayer& state, const RPGData& bookkeeping) override;
};

} } // namespaces
