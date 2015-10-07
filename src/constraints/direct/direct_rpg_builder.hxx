
#pragma once

#include <fs0_types.hxx>
#include <atom.hxx>
#include <constraints/direct/csp_handler.hxx>
#include <languages/fstrips/language.hxx>

namespace fs = fs0::language::fstrips;


namespace fs0 {

class State; class RelaxedState;

/**
 * 
 */
class DirectRPGBuilder {
public:
	//! Factory method to create Direct RPG builders
	static std::shared_ptr<DirectRPGBuilder> create(const std::vector<fs::AtomicFormula::cptr>& goalConditions, const std::vector<fs::AtomicFormula::cptr>& stateConstraints);
	
	DirectRPGBuilder(const std::vector<DirectConstraint::cptr>&& goalConstraints, const std::vector<DirectConstraint::cptr>&& stateConstraints);
	~DirectRPGBuilder();
	
	//! Prunes the domains contained in the state by filtering them with the state constraints.
	FilteringOutput pruneUsingStateConstraints(RelaxedState& state) const;


	//! Returns true iff the given RelaxedState is a goal according to the goal, state and goal constraints.
	//! Besides, return the causes of the goal to be later processed by the RPG heuristic backchaining procedure.
	bool isGoal(const State& seed, const RelaxedState& state, Atom::vctr& causes) const;
	
	//! This is a simplified version in which we don't care about causes, etc. but only about whether the layer is a goal or not.
	bool isGoal(const RelaxedState& state) const;
	
	friend std::ostream& operator<<(std::ostream &os, const DirectRPGBuilder& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
protected:
	//! We store here all the constraints that we want to take into account when determining if a relaxed state is a goal:
	//! This includes both the explicit goal constraints plus the state constraints.
	const std::vector<DirectConstraint::cptr> _stateConstraints;
	const std::vector<DirectConstraint::cptr> _allGoalConstraints;
	
	const DirectCSPHandler _stateConstraintsHandler;
	const DirectCSPHandler _goalConstraintsHandler;
	
	//! Returns true iff the given domains are not inconsistent when filtering them with all the goal constraints.
	bool checkGoal(const DomainMap& domains) const;
	
	//! Extract the supporters of the goal from the pruned domains and add them to the set of goal causes.
	//! If any pruned domain is empty, return false, as it means we have an inconsistency.
	void extractGoalCauses(const State& seed, const DomainMap& domains, const DomainMap& clone, Atom::vctr& causes, std::vector<bool>& set, unsigned num_set) const;

	void extractGoalCausesArbitrarily(const State& seed, const DomainMap& domains, Atom::vctr& causes, std::vector<bool>& set) const;
};



} // namespaces
