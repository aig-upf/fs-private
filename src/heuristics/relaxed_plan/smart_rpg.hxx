
#pragma once

#include <fs_types.hxx>
#include <constraints/gecode/extensions.hxx>
#include <constraints/gecode/handlers/lifted_formula_handler.hxx>
#include <utils/tuple_index.hxx>
#include <unordered_set>

namespace fs0 { class Problem; class State; class RPGData; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class EffectSchemaCSPHandler;
class RPGIndex;

class SmartRPG {
protected:
	typedef std::shared_ptr<EffectSchemaCSPHandler> EffectHandlerPtr;
	
public:
	SmartRPG(const Problem& problem, const fs::Formula* goal_formula, const fs::Formula* state_constraints);
	virtual ~SmartRPG() = default;
	
	// Disallow copies of the object, as they will be expensive, but allow moves.
	SmartRPG(const SmartRPG&) = delete;
	SmartRPG(SmartRPG&&) = default;
	SmartRPG& operator=(const SmartRPG& other) = delete;
	SmartRPG& operator=(SmartRPG&& other) = default;
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);
	
	//! The computation of the heuristic value. Returns -1 if the RPG layer encoded in the relaxed state is not a goal,
	//! otherwise returns h_{FF}.
	//! To be subclassed in other RPG-based heuristics such as h_max
	virtual long computeHeuristic(const RPGIndex& graph);
	
	void set_managers(std::vector<EffectHandlerPtr>&& managers) { _managers = std::move(managers); }

protected:
	//! The actual planning problem
	const Problem& _problem;
	const ProblemInfo& _info;
	
	const TupleIndex& _tuple_index;
	
	//! The set of action managers, one per every action
	std::vector<EffectHandlerPtr> _managers;
	
	//!
	ExtensionHandler _extension_handler;
	
	std::unique_ptr<LiftedFormulaHandler> _goal_handler;
};

} } // namespaces
