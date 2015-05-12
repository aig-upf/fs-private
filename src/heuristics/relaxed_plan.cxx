
#include <action_manager.hxx>
#include <relaxed_action_manager.hxx>
#include <fwd_search_prob.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <utils/projections.hxx>
#include <heuristics/rpg.hxx>

namespace fs0 {

template <typename T>
RelaxedPlanHeuristic<T>::RelaxedPlanHeuristic( const T& problem ) :
	Heuristic<State>( problem ),
	_problem(problem.getTask())
{}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
template <typename T>
float RelaxedPlanHeuristic<T>::evaluate(const State& seed) {
	
	if (_problem.isGoal(seed)) return 0; // The seed state is a goal
	
	const Action::vcptr& actions = _problem.getAllActions();
	RelaxedState relaxed(seed);
	std::vector<Changeset::ptr> changesets;
	
	#ifdef FS0_DEBUG
	std::cout << std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************" << std::endl;;
	#endif
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the changeset is empty),
	// or we get to a goal graph layer.
	while(true) {
		Changeset::ptr changeset = std::make_shared<Changeset>(seed, relaxed);
		
		// Apply all the actions to the RPG layer
		for (unsigned idx = 0; idx < actions.size(); ++idx) {
			const Action& action = *actions[idx];
			action.getConstraintManager()->processAction(idx, action, seed, relaxed, *changeset);
		}
		
		
		#ifdef FS0_DEBUG
		std::cout << std::endl << "Changeset size: " << changeset->size() << std::endl;
		std::cout << "Changeset: " << *changeset << std::endl;
		#endif
		
		// If there is no novel fact in the changeset, we reached a fixpoint, thus there is no solution.
		if (changeset->size() == 0) return std::numeric_limits<float>::infinity();
		
		changesets.push_back(changeset);
		
		unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		relaxed.accumulate(*changeset);
		
		// Prune using state constraints
		ScopedConstraint::Output o = _problem.getConstraintManager()->pruneUsingStateConstraints(relaxed);
		#ifdef FS0_DEBUG
		std::cout << "State Constraint pruning output: " <<  static_cast<std::underlying_type<ScopedConstraint::Output>::type>(o) << std::endl;
		#endif
		if (o == ScopedConstraint::Output::Failure) return std::numeric_limits<float>::infinity();
		if (o == ScopedConstraint::Output::Pruned && relaxed.getNumberOfAtoms() <= prev_number_of_atoms) return std::numeric_limits<float>::infinity();
		
		#ifdef FS0_DEBUG
		std::cout << "RPG Layer #" << changesets.size() << ": " << relaxed << std::endl; // std::cout << "Changeset: " << *changeset << std::endl << std::endl;
		#endif
		
		float h = computeHeuristic(seed, relaxed, changesets);
		if (h > -1) {
			return h;
		}
	}
}

template <typename T>
float RelaxedPlanHeuristic<T>::computeHeuristic(const State& seed, const RelaxedState& state, const Changeset::vptr& changesets) {
	Fact::vctr causes;
	if (_problem.getConstraintManager()->isGoal(seed, state, causes)) {
		RPGraph rpg = RPGraph(seed, changesets);
		auto cost = rpg.computeRelaxedPlanCost(causes);
		return cost;
	} else {
		return -1;
	}
}

template <typename T>
void RelaxedPlanHeuristic<T>::print_changesets(const std::vector<Changeset::ptr>& changesets) {
	for (unsigned i = 0; i < changesets.size(); ++i) {
		std::cout << "Layer #" << i << " changeset: " << std::endl;
		std::cout << *changesets[i] << std::endl;
	}
}

// explicit instantiations
template class RelaxedPlanHeuristic<FwdSearchProblem>;

} // namespaces

