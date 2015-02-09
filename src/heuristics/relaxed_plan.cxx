
#include <fwd_search_prob.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <utils/projections.hxx>
#include <action_manager.hxx>
#include <relaxed_applicability_manager.hxx>
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
	
	const Action::vcptr& actions = _problem.getAllActions();
	
	RelaxedState relaxed(seed);
	const RelaxedApplicabilityManager& appManager = _problem.getRelaxedApplicabilityManager();
	const RelaxedEffectManager& effManager = _problem.getRelaxedEffectManager();
	
	if (_problem.isGoal(seed)) return 0; // The seed state is a goal
	
	std::vector<Changeset::ptr> changesets;
	
	#ifdef FS0_DEBUG
	std::cout << std::endl << "RP Graph computation from seed state: " << std::endl << seed << std::endl << "****************************************" << std::endl;;
	#endif
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the changeset is empty),
	// or we get to a goal graph layer.
	while(true) {
		Changeset::ptr changeset = std::make_shared<Changeset>(seed, relaxed);
		
		for (unsigned idx = 0; idx < actions.size(); ++idx) {
			auto& action = *actions[idx];
			
			// We compute the projection of the current relaxed state to the variables relevant to the action
			// Note that this _clones_ the actual domains, since we want to modify them.
			DomainMap projection = Projections::projectToActionVariables(relaxed, action);
			
			// ... and this prunes them with the constraints represented by each procedure.
			Fact::vctr causes;
			if (appManager.checkPreconditionsHold(action, seed, projection, causes)) { // If the action is applicable in the current RPG layer...
				// ...we accumulate the effects on the changeset with all new reachable effects.
				changeset->setCurrentAction(idx, RPGraph::pruneSeedSupporters(causes, seed));  // We record the applicability causes
				effManager.computeChangeset(action, projection, *changeset);
			}
		}
		
		
		#ifdef FS0_DEBUG
		std::cout << std::endl << "Changeset size: " << changeset->size() << std::endl;
		// std::cout << "Changeset: " << *changeset << std::endl;
		// print_changesets(changesets);
		#endif
		
		// If there is no novel fact in the changeset, we reached a fixpoint, thus there is no solution.
		if (changeset->size() == 0) return std::numeric_limits<float>::infinity();
		
		changesets.push_back(changeset);
		
		unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		relaxed.accumulate(*changeset);
		
		// Prune using state constraints
		ScopedConstraint::Output o = _problem.getConstraintManager()->pruneUsingStateConstraints(relaxed);
// 		std::cout << "State Constraint pruning output: " <<  static_cast<std::underlying_type<ScopedConstraint::Output>::type>(o) << std::endl;
		if (o == ScopedConstraint::Output::Failure) return std::numeric_limits<float>::infinity();
		if (o == ScopedConstraint::Output::Pruned && relaxed.getNumberOfAtoms() <= prev_number_of_atoms) return std::numeric_limits<float>::infinity();
		
		#ifdef FS0_DEBUG
		std::cout << "New RPG Layer: " << relaxed << std::endl; // std::cout << "Changeset: " << *changeset << std::endl << std::endl;
		#endif
		
		float h = computeHeuristic(seed, relaxed, changesets);
		if (h > -1) {
			#ifdef FS0_DEBUG
			std::cout << "All changesets: " <<  std::endl; print_changesets(changesets);
			#endif
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

