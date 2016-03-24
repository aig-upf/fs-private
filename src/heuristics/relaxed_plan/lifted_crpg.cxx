
#include <limits>

#include <languages/fstrips/effects.hxx>
#include <heuristics/relaxed_plan/lifted_crpg.hxx>
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
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <languages/fstrips/scopes.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace gecode {

LiftedCRPG::LiftedCRPG(const Problem& problem, std::vector<EffectHandlerPtr>&& managers, std::vector<IndexedTupleset>&& symbol_tuplesets, std::shared_ptr<GecodeRPGBuilder> builder) :
	_problem(problem),
	_info(problem.getProblemInfo()),
	_managers(std::move(managers)),
	_builder(builder),
	_extension_handler(),
	_symbol_tuplesets(std::move(symbol_tuplesets))
// 	_atom_table(index_atoms(_info)),
// 	_atom_achievers(build_achievers_index(_managers, _atom_table)),
// 	_atom_variable_tuples(index_variable_tuples(_info, _atom_table)),
// 	_symbol_tuplesets(index_tuples(compute_all_reachable_tuples(_info), _info))
// 	index_tuplesets
{
	FDEBUG("heuristic", "Relaxed Plan heuristic initialized with builder: " << std::endl << *_builder);
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long LiftedCRPG::evaluate(const State& seed) {
	
	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal
	
	FFDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	GecodeRPGLayer layer(_extension_handler, seed);
	RPGData bookkeeping(seed, true); // We ignore negated atoms
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_builder->init_value_selector(&bookkeeping);
	}
	
	// Copy the sets of possible tuples and prune the ones corresponding to the seed state
// 	std::vector<Tupleset> tuplesets(_all_tuples_by_symbol);
// 	prune_tuplesets(seed, tuplesets);
	
	std::vector<std::set<unsigned>> reached_by_symbol = compute_reached_tuples(seed);
	
	while (true) {
		
		// Build a new layer of the RPG.

		for (const EffectHandlerPtr manager:_managers) {
			unsigned affected_symbol = manager->get_lhs_symbol();
// 			IndexedTupleset& tupleset = _symbol_tuplesets.at(affected_symbol);
			
			
			std::set<unsigned>& reached = reached_by_symbol.at(affected_symbol);  // We want a reference because we'll want to add newly-reached atoms.
			manager->seek_novel_tuples(layer, reached, bookkeeping, seed); // This will update 'reached' with newly-reached atoms
			
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

long LiftedCRPG::computeHeuristic(const State& seed, const GecodeRPGLayer& layer, const RPGData& rpg) {
	std::vector<Atom> causes;
	if (_builder->isGoal(seed, layer, causes)) {
		auto extractor = RelaxedPlanExtractorFactory<RPGData>::create(seed, rpg);
		long cost = extractor->computeRelaxedPlanCost(causes);
		delete extractor;
		return cost;
	} else return -1;
}

/*
Index<Atom> LiftedCRPG::index_atoms(const ProblemInfo& info) {
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
*/

/*
std::vector<std::vector<ObjectIdx>> LiftedCRPG::index_variable_tuples(const ProblemInfo& info, const Index<Atom>& index) {
	std::vector<std::vector<ObjectIdx>> tuples;
	for (const Atom& atom:index.elements()) {
		const auto& data = info.getVariableData(atom.getVariable());
		tuples.push_back(data.second);
	}
	return tuples;
}
*/

/*
LiftedCRPG::AchieverIndex LiftedCRPG::build_achievers_index(const std::vector<EffectHandlerPtr>& managers, const Index<Atom>& atom_idx) {
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
*/

/*
std::vector<LiftedCRPG::EffectHandlerPtr> LiftedCRPG::prepare_managers(const std::vector<ActionHandlerPtr>& managers) {
	std::vector<EffectHandlerPtr> downcasted;
	for (ActionHandlerPtr manager:managers) {
		auto effect_manager = std::dynamic_pointer_cast<EffectSchemaCSPHandler>(manager);
		if (!effect_manager) throw std::runtime_error("Currently only ground effect managers accepted");
		downcasted.push_back(effect_manager);
	}
	return downcasted;
}
*/

/*
void LiftedCRPG::prune_tuplesets(const State& seed, std::vector<gecode::Tupleset>& tuplesets) {
	const ProblemInfo& info = Problem::getInfo();
	
	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		const auto& data = info.getVariableData(var);
		auto& tupleset = tuplesets.at(data.first); // The tupleset corresponding to the symbol index
		
		if (info.isPredicativeVariable(var)) {
			if (seed.getValue(var)) {
				std::vector<int> arguments(data.second); // Copy the vector
				arguments.push_back(1); // We're just interested in the non-negated atom
				tupleset.remove(std::move(arguments));
			}
		} else {
			std::vector<int> arguments(data.second); // Copy the vector
			arguments.push_back(seed.getValue(var));
			tupleset.remove(std::move(arguments)); 
		}
	}
}
*/


// TODO - We should be applying some reachability analysis here to prune out tuples that will never be reachable at all.
std::vector<IndexedTupleset::TupleVector> LiftedCRPG::compute_all_reachable_tuples(const ProblemInfo& info) {
	std::vector<IndexedTupleset::TupleVector> tuples_by_symbol(info.getNumLogicalSymbols());

	for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
		const auto& data = info.getVariableData(var);
		auto& symbol_tuples = tuples_by_symbol.at(data.first); // The tupleset corresponding to the symbol index
		
		if (info.isPredicativeVariable(var)) {
			symbol_tuples.push_back(data.second); // We're just interested in the non-negated atom
		
			
		} else { // A function symbol
			for (ObjectIdx value:info.getVariableObjects(var)) {
				std::vector<int> arguments(data.second); // Copy the vector
				arguments.push_back(value);
				symbol_tuples.push_back(std::move(arguments)); 
			}
		}
	}
	return tuples_by_symbol;
}

std::vector<IndexedTupleset> LiftedCRPG::index_tuplesets(const ProblemInfo& info) {
	auto all_tuples = compute_all_reachable_tuples(info);
	std::vector<IndexedTupleset> tuplesets;
	for (unsigned symbol = 0; symbol < info.getNumLogicalSymbols(); ++symbol) {
		tuplesets.push_back(IndexedTupleset(all_tuples.at(symbol)));
	}
	return tuplesets;
}

std::vector<std::set<unsigned>> LiftedCRPG::compute_reached_tuples(const State& seed) const {
	std::vector<std::set<unsigned>> reached_by_symbol(_info.getNumLogicalSymbols());
	
	for (VariableIdx var = 0; var < _info.getNumVariables(); ++var) {
		const auto& data = _info.getVariableData(var);
		
		const IndexedTupleset& tupleset = _symbol_tuplesets.at(data.first);
		std::set<unsigned>& set = reached_by_symbol.at(data.first); // The set of reached symbols corresponding to the symbol index
		
		if (_info.isPredicativeVariable(var)) {
			if (seed.getValue(var)) { // We're just interested in non-negated atoms
				const auto& tuple = data.second;
				set.insert(tupleset.get_index(tuple));
			}
		} else {
			std::vector<int> tuple(data.second); // Copy the vector
			tuple.push_back(seed.getValue(var));
			set.insert(tupleset.get_index(tuple));
		}
	}
	
	return reached_by_symbol;
}

} } // namespaces

