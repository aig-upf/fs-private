
#pragma once

#include <fs0_types.hxx>
#include <atoms.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/filtering.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 { class State; class RelaxedState; }

namespace fs0 { namespace gecode {
	
//!
class GecodeRPGBuilder {
public:
	typedef GecodeRPGBuilder* cptr;
	
	//! Factory method - pointer ownership corresponds to the caller.
	static GecodeRPGBuilder::cptr create(const std::vector<fs::AtomicFormula::cptr>& goal_conditions, const std::vector<fs::AtomicFormula::cptr>& state_constraints);
	
	~GecodeRPGBuilder();
	
	//! Prunes the domains contained in the state by filtering them with the state constraints.
	FilteringOutput pruneUsingStateConstraints(RelaxedState& state) const;
	
	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	bool isGoal(const RelaxedState& state) const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeRPGBuilder& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
protected:
	//! Private constructor
	GecodeRPGBuilder(gecode::GecodeFormulaCSPHandler::ptr goal_handler, gecode::GecodeFormulaCSPHandler::ptr state_constraint_handler)
		: _goal_handler(goal_handler), _state_constraint_handler(state_constraint_handler) {}

	//! We need separate Formula CSPs for handling the goal and the state constraints.
	GecodeFormulaCSPHandler::ptr _goal_handler;
	GecodeFormulaCSPHandler::ptr _state_constraint_handler;
};

} } // namespaces

