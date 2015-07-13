
#pragma once

#include <cassert>
#include <iosfwd>
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <constraints/constraint_manager.hxx>

namespace fs0 {

	
class BaseConstraintManager {
public:
	typedef BaseConstraintManager* cptr;
	
	BaseConstraintManager() {}
	virtual ~BaseConstraintManager() {}
	
	virtual ScopedConstraint::Output pruneUsingStateConstraints(RelaxedState& state) const = 0;
	
	//! Goal checking for non-relaxed states.
	virtual inline bool isGoal(const State& s) const = 0;
	
	virtual inline unsigned numUnsatisfiedGoals( const State& s ) const  = 0 ;

	virtual inline unsigned numGoalConstraints( ) const = 0;

	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	virtual bool isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const = 0;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	virtual bool isGoal(const RelaxedState& state) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const BaseConstraintManager& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};


/**
 * 
 */
class PlanningConstraintManager : public BaseConstraintManager
{
public:
	typedef PlanningConstraintManager* cptr;
	
	PlanningConstraintManager(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints);
	
	~PlanningConstraintManager() {}
	
	//! Prunes the domains contained in the state by filtering them with the state constraints.
	ScopedConstraint::Output pruneUsingStateConstraints(RelaxedState& state) const;
	
	//! Goal checking for non-relaxed states.
	inline bool isGoal(const State& s) const { return goalConstraintsManager.checkSatisfaction(s); }
	
	inline unsigned numUnsatisfiedGoals( const State& s ) const { return goalConstraintsManager.countUnsatisfied(s); }

	inline unsigned numGoalConstraints( ) const { return goalConstraintsManager.numConstraints(); }

	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	bool isGoal(const RelaxedState& state) const;
	
	std::ostream& print(std::ostream& os) const {
		os << "[Basic Goal Constraint Manager]";
		return os;
	}
	
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
	void extractGoalCauses(const State& seed, const DomainMap& domains, const DomainMap& clone, Atom::vctr& causes, std::vector<bool>& set, unsigned num_set) const;

	void extractGoalCausesArbitrarily(const State& seed, const DomainMap& domains, Atom::vctr& causes, std::vector<bool>& set) const;
};


class PlanningConstraintManagerFactory {
public:
	//! A factory method to create the appropriate manager.
	static BaseConstraintManager* create(const ScopedConstraint::vcptr& goalConstraints, const ScopedConstraint::vcptr& stateConstraints);
};

} // namespaces

