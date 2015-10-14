
#include <limits>

#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/relaxed_plan_extractor.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <constraints/filtering.hxx>
#include <utils/logging.hxx>
#include <utils/printers/actions.hxx>
#include <relaxed_state.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <state_model.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


namespace fs0 { namespace gecode {

GecodeCRPG::GecodeCRPG(const FS0StateModel& model, std::vector<std::shared_ptr<GecodeManager>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder)
	: _problem(model.getTask()), _managers(std::move(managers)), _builder(std::move(builder))
{
	FDEBUG("heuristic", "Relaxed Plan heuristic initialized with builder: " << std::endl << *_builder);
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long GecodeCRPG::evaluate(const State& seed) {
	
	if (ApplicabilityManager::checkFormulaHolds(_problem.getGoalConditions(), seed)) return 0; // The seed state is a goal
	
	GecodeRPGLayer layer(seed);
	RPGData bookkeeping(seed);
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_builder->init_value_selector(&bookkeeping);
	}
	
	FFDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the rpg is empty), or we reach a goal layer.
	for (unsigned i = 0; ; ++i) {
		// Apply all the actions to the RPG layer
		for (const std::shared_ptr<GecodeManager>& manager:_managers) {
			if (i == 0 && Config::instance().useMinHMaxActionValueSelector()) { // We initialize the value selector only once
				manager->init_value_selector(&bookkeeping);
			}
			manager->process(seed, layer, bookkeeping);
		}
		
		FFDEBUG("heuristic", "The last layer of the RPG contains " << bookkeeping.getNumNovelAtoms() << " novel atoms." << std::endl << bookkeeping);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (bookkeeping.getNumNovelAtoms() == 0) return -1;
		
		// unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		layer.accumulate(bookkeeping.getNovelAtoms());
		FFDEBUG("heuristic", "RPG Layer #" << bookkeeping.getCurrentLayerIdx() << ": " << layer);
/*
 * RETHINK HOW TO FIT THE STATE CONSTRAINTS INTO THE CSP MODEL
 		
		// Prune using state constraints - TODO - Would be nicer if the whole state constraint pruning was refactored into a single line
		FilteringOutput o = _builder->pruneUsingStateConstraints(relaxed);
		FFDEBUG("heuristic", "State Constraint pruning output: " <<  static_cast<std::underlying_type<FilteringOutput>::type>(o));
		if (o == FilteringOutput::Failure) return std::numeric_limits<unsigned>::infinity();
		if (o == FilteringOutput::Pruned && relaxed.getNumberOfAtoms() <= prev_number_of_atoms) return std::numeric_limits<float>::infinity();
*/
		
		long h = computeHeuristic(seed, layer, bookkeeping);
		if (h > -1) return h;
		
		bookkeeping.advanceLayer();
	}
}

long GecodeCRPG::computeHeuristic(const State& seed, const GecodeRPGLayer& layer, const RPGData& rpg) {
	std::vector<Atom> causes;
	if (_builder->isGoal(seed, layer, causes)) {
		auto extractor = RelaxedPlanExtractorFactory<RPGData>::create(seed, rpg);
		long cost = extractor->computeRelaxedPlanCost(causes);
		delete extractor;
		return cost;
	} else return -1;
}

} } // namespaces

