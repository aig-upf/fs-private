
#include <limits>

#include <languages/fstrips/effects.hxx>
#include <heuristics/relaxed_plan/gecode_crpg2.hxx>
#include <heuristics/relaxed_plan/relaxed_plan_extractor.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <constraints/filtering.hxx>
#include <utils/logging.hxx>
#include <utils/printers/actions.hxx>
#include <relaxed_state.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <applicability/formula_interpreter.hxx>
#include <constraints/gecode/handlers/base_action_handler.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <languages/fstrips/scopes.hxx>


namespace fs0 { namespace gecode {

ConstrainedRPG::ConstrainedRPG(const Problem& problem, std::vector<ActionHandlerPtr>&& managers, std::shared_ptr<GecodeRPGBuilder> builder) :
	_problem(problem),
	_managers(std::move(managers)),
	_builder(std::move(builder)),
	_extension_handler(),
	_atom_idx(index_atoms(problem.getProblemInfo())),
	_atom_achievers(build_achievers_index(_managers, _atom_idx))
{
	FDEBUG("heuristic", "Relaxed Plan heuristic initialized with builder: " << std::endl << *_builder);
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long ConstrainedRPG::evaluate(const State& seed) {
	
	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal
	
	GecodeRPGLayer layer(_extension_handler, seed);
	RPGData bookkeeping(seed);
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_builder->init_value_selector(&bookkeeping);
	}
	
	FFDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the rpg is empty), or we reach a goal layer.
	for (unsigned i = 0; ; ++i) {
		// Apply all the actions to the RPG layer
		for (const ActionHandlerPtr& manager:_managers) {
			if (i == 0 && Config::instance().useMinHMaxActionValueSelector()) { // We initialize the value selector only once
				manager->init_value_selector(&bookkeeping);
			}
			manager->process(seed, layer, bookkeeping);
		}
		
		FFDEBUG("heuristic", "The last layer of the RPG contains " << bookkeeping.getNumNovelAtoms() << " novel atoms." << std::endl << bookkeeping);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (bookkeeping.getNumNovelAtoms() == 0) return -1;
		
		// unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		layer.advance(bookkeeping.getNovelAtoms());
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

long ConstrainedRPG::computeHeuristic(const State& seed, const GecodeRPGLayer& layer, const RPGData& rpg) {
	std::vector<Atom> causes;
	if (_builder->isGoal(seed, layer, causes)) {
		auto extractor = RelaxedPlanExtractorFactory<RPGData>::create(seed, rpg);
		long cost = extractor->computeRelaxedPlanCost(causes);
		delete extractor;
		return cost;
	} else return -1;
}

Index<Atom> ConstrainedRPG::index_atoms(const ProblemInfo& info) {
	// TODO Take into account ONLY those atoms which are reachable !!!
	Index<Atom> index;
	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		for (ObjectIdx value:info.getVariableObjects(var)) {
			index.add(Atom(var, value));
		}
	}
	return index;
}


ConstrainedRPG::AchieverIndex ConstrainedRPG::build_achievers_index(const std::vector<ActionHandlerPtr>& managers, const Index<Atom>& atom_idx) {
	AchieverIndex index(atom_idx.size()); // Create an index as large as the number of atoms
	
	FINFO("main", "Building index of potential atom achievers");
	
	for (const ActionHandlerPtr& manager:managers) {
		auto effect_manager = std::dynamic_pointer_cast<GroundEffectCSPHandler>(manager);
		if (!effect_manager) throw std::runtime_error("Currently only ground effect managers accepted");
		
		auto effects = manager->get_effects();
		assert(effects.size() == 1);
		const fs::ActionEffect* effect = effects[0];
		
		// TODO - This uses a very rough overapproximation of the set of potentially affected atoms.
		// A better approach would be to build once, from the initial state, the full RPG, 
		// and extract from there, for each action / effect CSP, the set of atoms
		// that are reached by the CSP in some layer of the RPG.
		
		for (const auto& atom:fs::ScopeUtils::compute_affected_atoms(effect)) {
			unsigned idx = atom_idx.index(atom);
			index.at(idx).push_back(manager);
		}
	}
	
	return index;
}

} } // namespaces

