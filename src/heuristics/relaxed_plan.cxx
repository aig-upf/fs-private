
#include <heuristics/relaxed_plan.hxx>
#include <state_model.hxx>
#include <utils/projections.hxx>
#include <heuristics/relaxed_plan_extractor.hxx>
#include <heuristics/rpg_data.hxx>
#include <constraints/filtering.hxx>
#include <utils/logging.hxx>
#include <relaxed_state.hxx>
#include <heuristics/rpg/base_action_manager.hxx>

#include <state_model.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


namespace fs0 {

template <typename Model, typename RPGBuilder>
RelaxedPlanHeuristic<Model, RPGBuilder>::RelaxedPlanHeuristic(const Model& problem, std::shared_ptr<RPGBuilder> builder) :
	_problem(problem.getTask()), _builder(builder)
{}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
template <typename Model, typename RPGBuilder>
float RelaxedPlanHeuristic<Model, RPGBuilder>::evaluate(const State& seed) {
	
	if (ApplicabilityManager::checkFormulaHolds(_problem.getGoalConditions(), seed)) return 0; // The seed state is a goal
	
	const std::vector<GroundAction::cptr>& actions = _problem.getGroundActions();
	
	RelaxedState relaxed(seed);
	RPGData rpgData(relaxed);
	
	FFDEBUG("main", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the rpg is empty),
	// or we get to a goal graph layer.
	while(true) {
		// Apply all the actions to the RPG layer
		for (unsigned idx = 0; idx < actions.size(); ++idx) {
			const GroundAction& action = *actions[idx];
			action.getManager()->process(idx, relaxed, rpgData);
		}
		
		FFDEBUG("main", std::endl << "The last layer of the RPG contains " << rpgData.getNovelAtoms().size() << " novel atoms." << std::endl << rpgData);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (rpgData.getNovelAtoms().size() == 0) return std::numeric_limits<float>::infinity();
		
		unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		RPGData::accumulate(relaxed, rpgData);
		rpgData.advanceLayer();
		
		// Prune using state constraints - TODO - Would be nicer if the whole state constraint pruning was refactored into a single line
		FilteringOutput o = _builder->pruneUsingStateConstraints(relaxed);
		FFDEBUG("main", "State Constraint pruning output: " <<  static_cast<std::underlying_type<FilteringOutput>::type>(o));
		if (o == FilteringOutput::Failure) return std::numeric_limits<float>::infinity();
		if (o == FilteringOutput::Pruned && relaxed.getNumberOfAtoms() <= prev_number_of_atoms) return std::numeric_limits<float>::infinity();
		
		
		FFDEBUG("main", "RPG Layer #" << rpgData.getCurrentLayerIdx() << ": " << relaxed);
		
		float h = computeHeuristic(seed, relaxed, rpgData);
		if (h > -1) return h;
	}
}

template <typename Model, typename RPGBuilder>
float RelaxedPlanHeuristic<Model, RPGBuilder>::computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpgData) {
	Atom::vctr causes;
	if (_builder->isGoal(seed, state, causes)) {
		BaseRelaxedPlanExtractor* extractor = RelaxedPlanExtractorFactory::create(seed, rpgData);
		float cost = extractor->computeRelaxedPlanCost(causes);
		delete extractor;
		return cost;
	} else return -1;
}

// explicit instantiations
template class RelaxedPlanHeuristic<fs0::FS0StateModel, fs0::DirectRPGBuilder>;
template class RelaxedPlanHeuristic<fs0::FS0StateModel, fs0::gecode::GecodeRPGBuilder>;

} // namespaces

