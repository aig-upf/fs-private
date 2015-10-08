
#pragma once

#include <fs0_types.hxx>
#include <constraints/gecode/action_manager.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <constraints/gecode/rpg_layer.hxx>

namespace fs0 { class GroundAction; class FS0StateModel; class Problem; class State;}

namespace fs0 { namespace gecode {

class GecodeCRPG {
public:
	typedef GroundAction Action;

	GecodeCRPG(const FS0StateModel& model, std::vector<std::shared_ptr<GecodeActionManager>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder);
	
	virtual ~GecodeCRPG() {}
	
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
	const std::vector<std::shared_ptr<GecodeActionManager>> _managers;
	
	//! The RPG building helper
	const std::shared_ptr<GecodeRPGBuilder> _builder;
};

} } // namespaces
