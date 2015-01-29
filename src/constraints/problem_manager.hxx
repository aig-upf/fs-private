
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <constraints/constraint_manager.hxx>

namespace fs0 {

/**
 * 
 */
class PlanningConstraintManager
{
public:
	typedef std::shared_ptr<const PlanningConstraintManager> cptr;
	
	PlanningConstraintManager(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints);
	
	//! Prunes the domains contained in the state by filtering them with the state constraints.
	ScopedConstraint::Output pruneUsingStateConstraints(RelaxedState& state) const;
	
	//! Goal checking for non-relaxed states.
	inline bool isGoal(const State& s) const { return goalConstraintsManager.checkSatisfaction(s); }
	
	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const RelaxedState& state, Fact::vctr& causes) const;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	bool isGoal(const RelaxedState& state) const;
	
protected:
	
	//! We store here all the constraints that we want to take into account when determining if a relaxed state is a goal:
	//! This includes both the explicit goal constraints plus the state constraints.
	const ScopedConstraint::vcptr allGoalConstraints;
	
	const ConstraintManager stateConstraintsManager;
	const ConstraintManager goalConstraintsManager;
	
	//! True iff there is at least one state constraint.
	bool hasStateConstraints;
	
	//! Returns true iff the given domains are not inconsistent when filtering them with all the goal constraints.
	bool checkGoal(const DomainMap& domains) const;
	
	//! Extract the supporters of the goal from the pruned domains and add them to the set of goal causes.
	//! If any pruned domain is empty, return false, as it means we have an inconsistency.
	void extractGoalCauses(const State& seed, const DomainMap& domains, const DomainMap& clone, Fact::vctr& causes, std::vector<bool>& set, unsigned num_set) const;

	void extractGoalCausesArbitrarily(const State& seed, const DomainMap& domains, Fact::vctr& causes, std::vector<bool>& set) const;
};

} // namespaces

