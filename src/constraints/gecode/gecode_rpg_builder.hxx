
#pragma once

#include <fs_types.hxx>
#include <atom.hxx>
#include <constraints/gecode/simple_csp.hxx>


namespace fs0 { class State; class RelaxedState; class TupleIndex; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class GecodeRPGLayer;
class FormulaCSPHandler;

//!
class GecodeRPGBuilder {
public:
	//! Factory method - pointer ownership corresponds to the caller.
	static std::shared_ptr<GecodeRPGBuilder> create(const fs::Formula* goal_formula, const fs::Formula* state_constraints, const TupleIndex& tuple_index);
	
	GecodeRPGBuilder(FormulaCSPHandler* goal_handler, FormulaCSPHandler* state_constraint_handler)
		: _goal_handler(goal_handler), _state_constraint_handler(state_constraint_handler) {}
	~GecodeRPGBuilder();
	
	//! Prunes the domains contained in the state by filtering them with the state constraints.
	// FilteringOutput pruneUsingStateConstraints(GecodeRPGLayer& state) const;
	
	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const GecodeRPGLayer& layer, Atom::vctr& causes) const;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	bool isGoal(const GecodeRPGLayer& layer) const;
	
	//! Initialize the value selector of the underlying CSPs
	void init_value_selector(const RPGData* bookkeeping);
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeRPGBuilder& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
protected:
	//! We need separate Formula CSPs for handling the goal and the state constraints.
	FormulaCSPHandler* _goal_handler;
	FormulaCSPHandler* _state_constraint_handler;
};

} } // namespaces

