
#include <limits>

#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <applicability/formula_interpreter.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <constraints/gecode/lifted_plan_extractor.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <problem.hxx>

namespace fs0 { namespace gecode {

SmartRPG::SmartRPG(const Problem& problem, const fs::Formula* goal_formula, const fs::Formula* state_constraints) :
	_problem(problem),
	_info(ProblemInfo::getInstance()),
	_tuple_index(problem.get_tuple_index()),
	_managers(),
	_extension_handler(_tuple_index),
	_goal_handler(std::unique_ptr<LiftedFormulaHandler>(new LiftedFormulaHandler(goal_formula->conjunction(state_constraints), _tuple_index, false)))
{
	FINFO("heuristic", "SmartRPG heuristic initialized");
}

//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long SmartRPG::evaluate(const State& seed) {
	
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

long SmartRPG::computeHeuristic(const State& seed, const RPGIndex& graph) {
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

} } // namespaces

