
#include <core_fwd_search_prob.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <utils/projections.hxx>
#include <action_manager.hxx>
#include <relaxed_action_set_manager.hxx>
#include <heuristics/rpg.hxx>

namespace aptk { namespace core {

template <typename T>
RelaxedPlanHeuristic<T>::RelaxedPlanHeuristic( const T& problem ) :
	Heuristic<State>( problem ),
	_problem(problem.getTask())
{}

//! The actual evaluation of the heuristic value for any given non-relaxed state s.
template <typename T>
float RelaxedPlanHeuristic<T>::evaluate(const State& seed) {
	
	const ProblemConstraint::vctr& stateConstraints = _problem.getConstraints();
	const ActionList& actions = _problem.getAllActions();
	
	RelaxedState relaxed(seed);
	RelaxedActionSetManager manager(&seed, stateConstraints);
	
	std::vector<Changeset::ptr> changesets;
	
	if (_problem.isGoal(seed)) return 0; // The seed state is a goal
	
	#ifdef FS0_DEBUG
	std::cout << std::endl << "RP Graph computation from seed state: " << std::endl << seed << std::endl << "****************************************" << std::endl;;
	#endif
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the changeset is empty),
	// or we get to a goal graph layer.
	while(true) {
		Changeset::ptr changeset = std::make_shared<Changeset>(&relaxed);
		
		for (unsigned idx = 0; idx < actions.size(); ++idx) {
			auto& action = *actions[idx];
			
			// We compute the projection of the current relaxed state to the variables relevant to the action
			// Note that this clones the actual domains...
			DomainMap projection = Projections::projectToActionVariables(relaxed, action);
			
			// ... and this prunes them with the unary constraints represented by each procedure.
			std::pair<bool, FactSetPtr> res = manager.isApplicable(action, projection);
			if (res.first) { // If the action is applicable in the current RPG layer...
				// ...we accumulate the effects on the changeset with all new reachable effects.
				changeset->setCurrentAction(idx, res.second);  // We record the applicability causes
				manager.computeChangeset(action, projection, *changeset);
			}
		}
		
		
		#ifdef FS0_DEBUG
		std::cout << "Changeset size: " << changeset->size() << std::endl;
		// print_changesets(changesets);
		// changeset->printEffects(std::cout);
		#endif
		
		// If there is no novel fact in the changeset, we reached a fixpoint, thus there is no solution.
		if (changeset->size() == 0) return std::numeric_limits<float>::infinity();
		
		changesets.push_back(changeset);
		
		unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		relaxed.accumulate(*changeset);
		
		// Prune using state constraints
		Constraint::Output o = _problem.getConstraintManager()->pruneUsingStateConstraints(relaxed);
// 		std::cout << "State Constraint pruning output: " <<  static_cast<std::underlying_type<Constraint::Output>::type>(o) << std::endl;
		if (o == Constraint::Output::Failure) return std::numeric_limits<float>::infinity();
		if (o == Constraint::Output::Pruned && relaxed.getNumberOfAtoms() <= prev_number_of_atoms) return std::numeric_limits<float>::infinity();
		
		#ifdef FS0_DEBUG
		std::cout << std::endl << "New RPG Layer: " << relaxed << std::endl; // std::cout << "Changeset: " << *changeset << std::endl << std::endl;
		#endif
		
		float h = computeHeuristic(seed, relaxed, changesets);
		if (h > -1) return h;
	}
}

template <typename T>
float RelaxedPlanHeuristic<T>::computeHeuristic(const State& seed, const RelaxedState& state, const std::vector<Changeset::ptr>& changesets) {
	FactSetPtr causes = std::make_shared<FactSet>();
	if (_problem.getConstraintManager()->isGoal(seed, state, causes)) {
		RPGraph rpg = RPGraph(_problem, seed, changesets);
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

} } // namespaces

