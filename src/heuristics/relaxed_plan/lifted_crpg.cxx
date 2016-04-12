
#include <limits>

#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <heuristics/relaxed_plan/lifted_crpg.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <applicability/formula_interpreter.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <constraints/gecode/lifted_plan_extractor.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <problem.hxx>

namespace fs0 { namespace gecode {

LiftedCRPG::LiftedCRPG(const Problem& problem, const fs::Formula* goal_formula, const fs::Formula* state_constraints) :
	_problem(problem),
	_info(problem.getProblemInfo()),
	_tuple_index(problem.get_tuple_index()),
	_managers(),
	_extension_handler(_tuple_index),
// 	_symbol_tuplesets(index_tuplesets(_info)),
	_goal_handler(std::unique_ptr<LiftedFormulaHandler>(new LiftedFormulaHandler(goal_formula->conjunction(state_constraints), _tuple_index, false)))
// 	_atom_table(index_atoms(_info)),
// 	_atom_achievers(build_achievers_index(_managers, _atom_table)),
// 	_atom_variable_tuples(index_variable_tuples(_info, _atom_table)),
{
	FINFO("heuristic", "LiftedCRPG heuristic initialized");
}

LiftedCRPG::~LiftedCRPG() {
}

//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long LiftedCRPG::evaluate(const State& seed) {
	
	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal
	
	FFDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	RPGIndex graph(seed, _tuple_index, _extension_handler);
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_goal_handler->init_value_selector(&graph);
	}
	
	// Copy the sets of possible tuples and prune the ones corresponding to the seed state
// 	std::vector<Tupleset> tuplesets(_all_tuples_by_symbol);
// 	prune_tuplesets(seed, tuplesets);
	
	while (true) {
		
		// Build a new layer of the RPG.
		for (const EffectHandlerPtr manager:_managers) {
			// TODO - RETHINK
// 			if (i == 0 && Config::instance().useMinHMaxActionValueSelector()) { // We initialize the value selector only once
// 				manager->init_value_selector(&bookkeeping);
// 			}	
// 			unsigned affected_symbol = manager->get_lhs_symbol();
			
			// If the effect has a fixed achievable tuple (e.g. because it is of the form X := c), and this tuple has already
			// been reached in the RPG, we can safely skip it.
			TupleIdx achievable = manager->get_achievable_tuple();
			if (achievable != INVALID_TUPLE && graph.reached(achievable)) continue;
			
			// Otherwise, we process the effect to derive the new tuples that it can produce on the current RPG layer
			manager->seek_novel_tuples(graph, seed);
		}
		
		
		// TODO - RETHINK HOW TO FIT THE STATE CONSTRAINTS INTO THIS CSP MODEL
		
// 		FFDEBUG("heuristic", "The last layer of the RPG contains " << graph.num_novel_tuples() << " novel atoms." << std::endl << graph);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (!graph.hasNovelTuples()) return -1;
		
		
		graph.advance(); // Integrates the novel tuples into the graph as a new layer.
		FFDEBUG("heuristic", "New RPG Layer: " << graph);
		
		long h = computeHeuristic(seed, graph);
		if (h > -1) return h;
		
	}
}

long LiftedCRPG::computeHeuristic(const State& seed, const RPGIndex& graph) {
	long cost = -1;
	if (SimpleCSP* csp = _goal_handler->instantiate(graph)) {
		if (csp->checkConsistency()) { // ATM we only take into account full goal resolution
			FFDEBUG("heuristic", "Goal formula CSP is consistent: " << *csp);
			std::vector<TupleIdx> causes;
			if (_goal_handler->compute_support(csp, causes, seed)) {
				LiftedPlanExtractor extractor(seed, graph, _tuple_index);
				cost = extractor.computeRelaxedPlanCost(causes);
			}
		}
		delete csp;
	}
	return cost;
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

/*
std::vector<IndexedTupleset> LiftedCRPG::index_tuplesets(const ProblemInfo& info) {
	auto all_tuples = TupleIndex::compute_all_reachable_tuples(info);
	std::vector<IndexedTupleset> tuplesets;
	for (unsigned symbol = 0; symbol < info.getNumLogicalSymbols(); ++symbol) {
		tuplesets.push_back(IndexedTupleset(all_tuples.at(symbol)));
	}
	return tuplesets;
}
*/



} } // namespaces

