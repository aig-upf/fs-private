
#include <limits>

#include <languages/fstrips/effects.hxx>
#include <heuristics/relaxed_plan/atom_based_crpg.hxx>
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

ConstrainedRPG::ConstrainedRPG(const Problem& problem, const std::vector<ActionHandlerPtr>& managers, std::shared_ptr<GecodeRPGBuilder> builder) :
	_problem(problem),
	_managers(downcast_managers(managers)),
	_builder(std::move(builder)),
	_extension_handler(),
	_atom_table(index_atoms(problem.getProblemInfo())),
	_atom_achievers(build_achievers_index(_managers, _atom_table)),
	_atom_variable_tuples(index_variable_tuples(problem.getProblemInfo(), _atom_table))
{
	FDEBUG("heuristic", "Relaxed Plan heuristic initialized with builder: " << std::endl << *_builder);
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long ConstrainedRPG::evaluate(const State& seed) {
	
	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal
	
	FFDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	GecodeRPGLayer layer(_extension_handler, seed);
	// TODO - We would ideally ignore negated atoms in the construction of the RPGData object by setting the second parameter to true,
	// but it seems that checking for each variable whether it is predicative or not... doesn't pay off (!)
	RPGData bookkeeping(seed, false);
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_builder->init_value_selector(&bookkeeping);
	}
	
	auto unachieved = layer.unachieved_atoms(_atom_table);
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the rpg is empty), or we reach a goal layer.
	while (true) {
	
		// Begin a new RPG Layer
		
		// cache[i] contains the CSP corresponding to effect 'i' instantiated to the current layer, or nullptr.
		// We use to avoid instantiating the same effect CSP more than once per layer
		std::vector<std::unique_ptr<SimpleCSP>> cache(_managers.size());
		std::vector<bool> failure_cache(_managers.size(), false);
		
		
		for (auto it = unachieved.begin(); it != unachieved.end(); ) {
			unsigned atom_idx = *it;
			const Atom& atom = _atom_table.element(atom_idx);
			
			// Check for a potential support
			bool atom_supported = false;
			for (unsigned manager_idx:_atom_achievers.at(atom_idx)) {
				const EffectHandlerPtr& manager = _managers[manager_idx];
				if (failure_cache[manager_idx]) {
					FFDEBUG("heuristic", "Found cached unapplicable effect \"" << *manager->get_effect() << "\" of action \"" << manager->get_action().fullname() << "\"");
					continue; // The effect CSP has already been instantiated and found unapplicable on this very same layer
				}
				
				if (cache[manager_idx] == nullptr) {
					SimpleCSP* raw = manager->preinstantiate(layer);
					if (!raw) { // We are instantiating the CSP for the first time in this layer and find that it is not applicable.
						failure_cache[manager_idx] = true;
						FFDEBUG("heuristic", "Effect \"" << *manager->get_effect() << "\" of action \"" << manager->get_action().fullname() << "\" inconsistent => not applicable");
						continue;
					}
					cache[manager_idx] = std::unique_ptr<SimpleCSP>(raw);
				} else {
					FFDEBUG("heuristic", "Found cached & applicable effect \"" << *manager->get_effect() << "\" of action \"" << manager->get_action().fullname() << "\"");
				}
				
				atom_supported = manager->find_atom_support(atom, seed, *cache[manager_idx], bookkeeping);
				if (atom_supported) break; // No need to keep iterating
			}
			
			// If a support was found, no need to check for that particular atom anymore.
			if (atom_supported) {
				FFDEBUG("heuristic", "Found support for atom " << atom);
				it = unachieved.erase(it);
			} else {
				++it;
			}
		}
		
		// TODO - RETHINK HOW TO FIT THE STATE CONSTRAINTS INTO THIS CSP MODEL
		
		FFDEBUG("heuristic", "The last layer of the RPG contains " << bookkeeping.getNumNovelAtoms() << " novel atoms." << std::endl << bookkeeping);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (bookkeeping.getNumNovelAtoms() == 0) return -1;
		
		// unsigned prev_number_of_atoms = relaxed.getNumberOfAtoms();
		layer.advance(bookkeeping.getNovelAtoms());
		FFDEBUG("heuristic", "RPG Layer #" << bookkeeping.getCurrentLayerIdx() << ": " << layer);
		
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
		if (info.isPredicativeVariable(var)) {
			index.add(Atom(var, 1)); // We don't need the negated atom, since the RPG ignores delete effects.
			continue;
		}
		
		for (ObjectIdx value:info.getVariableObjects(var)) {
			index.add(Atom(var, value));
		}
	}
	return index;
}

std::vector<std::vector<ObjectIdx>> ConstrainedRPG::index_variable_tuples(const ProblemInfo& info, const Index<Atom>& index) {
	std::vector<std::vector<ObjectIdx>> tuples;
	for (const Atom& atom:index.elements()) {
		const auto& data = info.getVariableData(atom.getVariable());
		tuples.push_back(data.second);
	}
	return tuples;
}

ConstrainedRPG::AchieverIndex ConstrainedRPG::build_achievers_index(const std::vector<EffectHandlerPtr>& managers, const Index<Atom>& atom_idx) {
	AchieverIndex index(atom_idx.size()); // Create an index as large as the number of atoms
	
	FINFO("main", "Building index of potential atom achievers");
	
	for (unsigned manager_idx = 0; manager_idx < managers.size(); ++manager_idx) {
		const EffectHandlerPtr& manager = managers[manager_idx];
		
		const fs::ActionEffect* effect = manager->get_effect();
		
		// TODO - This uses a very rough overapproximation of the set of potentially affected atoms.
		// A better approach would be to build once, from the initial state, the full RPG, 
		// and extract from there, for each action / effect CSP, the set of atoms
		// that are reached by the CSP in some layer of the RPG.
		
		for (const auto& atom:fs::ScopeUtils::compute_affected_atoms(effect)) {
			unsigned idx = atom_idx.index(atom);
			index.at(idx).push_back(manager_idx);
		}
	}
	
	return index;
}

std::vector<ConstrainedRPG::EffectHandlerPtr> ConstrainedRPG::downcast_managers(const std::vector<ActionHandlerPtr>& managers) {
	std::vector<EffectHandlerPtr> downcasted;
	for (ActionHandlerPtr manager:managers) {
		auto effect_manager = std::dynamic_pointer_cast<GroundEffectCSPHandler>(manager);
		if (!effect_manager) throw std::runtime_error("Currently only ground effect managers accepted");
		downcasted.push_back(effect_manager);
	}
	return downcasted;
}


} } // namespaces

