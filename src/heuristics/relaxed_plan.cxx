
#include <action_manager.hxx>
#include <relaxed_action_manager.hxx>
#include <fwd_search_prob.hxx>
#include <heuristics/relaxed_plan.hxx>
#include <utils/projections.hxx>
#include <heuristics/rpg.hxx>
#include <heuristics/rpg_data.hxx>
#include <utils/logging.hxx>

namespace fs0 {

template <typename T>
RelaxedPlanHeuristic<T>::RelaxedPlanHeuristic( const T& problem ) :
	_problem(problem.getTask())
{}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
template <typename T>
float RelaxedPlanHeuristic<T>::evaluate(const State& seed) {
	
	if (_problem.isGoal(seed)) { // The seed state is a goal
		return 0;
	}
	
	const typename Action::vcptr& actions = _problem.getAllActions();
	RelaxedState relaxed(seed);
	RPGData rpgData(relaxed);
	
	FFDEBUG("main", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the rpg is empty),
	// or we get to a goal graph layer.
	while(true) {
		// Apply all the actions to the RPG layer
		for (unsigned idx = 0; idx < actions.size(); ++idx) {
			const Action& action = *actions[idx];
			action.getConstraintManager()->processAction(idx, action, relaxed, rpgData);
		}
		
		FFDEBUG("main", std::endl << "The last layer of the RPG contains " << rpgData.getNovelAtoms().size() << " novel atoms." << std::endl << rpgData);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (rpgData.getNovelAtoms().size() == 0) return std::numeric_limits<float>::infinity();
		
		unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		RPGData::accumulate(relaxed, rpgData);
		rpgData.advanceLayer();
		
		// Prune using state constraints
		ScopedConstraint::Output o = _problem.getConstraintManager()->pruneUsingStateConstraints(relaxed);
		
		FFDEBUG("main", "State Constraint pruning output: " <<  static_cast<std::underlying_type<ScopedConstraint::Output>::type>(o));
		if (o == ScopedConstraint::Output::Failure) return std::numeric_limits<float>::infinity();
		if (o == ScopedConstraint::Output::Pruned && relaxed.getNumberOfAtoms() <= prev_number_of_atoms) return std::numeric_limits<float>::infinity();
		
		
		FFDEBUG("main", "RPG Layer #" << rpgData.getCurrentLayerIdx() << ": " << relaxed);
		
		float h = computeHeuristic(seed, relaxed, rpgData);
		if (h > -1) {
			return h;
		}
	}
}

template <typename T>
float RelaxedPlanHeuristic<T>::computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpgData) {
	Atom::vctr causes;
	if (_problem.getConstraintManager()->isGoal(seed, state, causes)) {
		RPGraph rpg = RPGraph(seed, rpgData);
		auto cost = rpg.computeRelaxedPlanCost(causes);
		return cost;
	} else {
		return -1;
	}
}


// explicit instantiations
template class RelaxedPlanHeuristic<FwdSearchProblem>;

} // namespaces

