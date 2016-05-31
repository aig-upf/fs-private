
#include <limits>

#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <heuristics/relaxed_plan/smart_rpg.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <heuristics/relaxed_plan/relaxed_plan.hxx>
#include <applicability/formula_interpreter.hxx>
#include <constraints/gecode/handlers/lifted_effect_csp.hxx>
#include <constraints/gecode/lifted_plan_extractor.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/config.hxx>
#include <problem.hxx>

namespace fs0 { namespace gecode {

SmartRPG::SmartRPG(const Problem& problem, const fs::Formula* goal_formula, const fs::Formula* state_constraints, std::vector<EffectHandlerPtr>&& managers, ExtensionHandler extension_handler) :
	_problem(problem),
	_info(ProblemInfo::getInstance()),
	_tuple_index(problem.get_tuple_index()),
	_managers(std::move(managers)),
	_extension_handler(extension_handler),
	_goal_handler(std::unique_ptr<FormulaCSP>(new FormulaCSP(goal_formula->conjunction(state_constraints), _tuple_index, false)))
{
	LPT_INFO("heuristic", "SmartRPG heuristic initialized");
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long SmartRPG::evaluate(const State& seed, std::vector<Atom>& relevant) {
	
	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal
	
	LPT_EDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	RPGIndex graph(seed, _tuple_index, _extension_handler);
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_goal_handler->init_value_selector(&graph);
	}
	
	while (true) {
		
		// Build a new layer of the RPG.
		for (const EffectHandlerPtr& manager:_managers) {
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
			manager->seek_novel_tuples(graph);
		}
		
		
		// TODO - RETHINK HOW TO FIT THE STATE CONSTRAINTS INTO THIS CSP MODEL
// 		LPT_EDEBUG("heuristic", "The last layer of the RPG contains " << graph.num_novel_tuples() << " novel atoms." << std::endl << graph);
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (!graph.hasNovelTuples()) return -1;
		
		
		graph.advance(); // Integrates the novel tuples into the graph as a new layer.
		LPT_EDEBUG("heuristic", "New RPG Layer: " << graph);
		
		long h = computeHeuristic(graph, relevant);
		if (h > -1) return h;
	}
}

long SmartRPG::computeHeuristic(const RPGIndex& graph, std::vector<Atom>& relevant) {
	return support::compute_rpg_cost(_tuple_index, graph, *_goal_handler, relevant);
}

//! Computes the full RPG until a fixpoint is reached, disregarding goal.
RPGIndex SmartRPG::compute_full_graph(const State& seed) {
	LPT_EDEBUG("heuristic", std::endl << "Computing Full RPG from seed state: " << std::endl << seed);
	
	RPGIndex graph(seed, _tuple_index, _extension_handler);
	
	// See method 'evaluate' for comments on the logic of this loop
	while (true) {
		LPT_EDEBUG("heuristic", "Opening layer of the full RPG");
		for (const EffectHandlerPtr& manager:_managers) {
			TupleIdx achievable = manager->get_achievable_tuple();
			if (achievable != INVALID_TUPLE && graph.reached(achievable)) continue;
			
			// Otherwise, we process the effect to derive the new tuples that it can produce on the current RPG layer
			manager->seek_novel_tuples(graph);
		}
		
		if (!graph.hasNovelTuples()) break;

		graph.advance(); // Integrates the novel tuples into the graph as a new layer.
		LPT_EDEBUG("heuristic", "New RPG Layer: " << graph);
	}
	
	LPT_EDEBUG("heuristic", "Full RPG is: " << graph);
	return graph;
}

} } // namespaces

