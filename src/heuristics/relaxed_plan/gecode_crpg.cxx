
#include <limits>

#include <languages/fstrips/language.hxx>
#include <languages/fstrips/operations.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/relaxed_plan_extractor.hxx>
#include <relaxed_state.hxx>
#include <applicability/formula_interpreter.hxx>
#include <constraints/gecode/handlers/base_action_csp.hxx>
#include <constraints/gecode/handlers/formula_csp.hxx>
#include <constraints/gecode/lifted_plan_extractor.hxx>
#include <heuristics/relaxed_plan/relaxed_plan.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>

namespace fs0 { namespace gecode {

GecodeCRPG::GecodeCRPG(const Problem& problem, const fs::Formula* goal_formula, const fs::Formula* state_constraints, std::vector<std::shared_ptr<BaseActionCSP>>&& managers, ExtensionHandler extension_handler) :
	_problem(problem),
	_tuple_index(problem.get_tuple_index()),
	_managers(std::move(managers)),
	_extension_handler(extension_handler),
	_goal_handler(std::unique_ptr<FormulaCSP>(new FormulaCSP(fs::conjunction(*goal_formula, *state_constraints), _tuple_index, false)))
{
	LPT_DEBUG("heuristic", "Standard CRPG heuristic initialized");
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long GecodeCRPG::evaluate(const State& seed, std::vector<Atom>& relevant) {
	
	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal
	
	RPGIndex graph(seed, _tuple_index, _extension_handler);
	
	if (Config::instance().useMinHMaxGoalValueSelector()) {
		_goal_handler->init_value_selector(&graph);
	}	
	
	LPT_EDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");
	
	// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the rpg is empty), or we reach a goal layer.
	for (unsigned i = 0; ; ++i) {
		// Apply all the actions to the RPG layer
		for (const std::shared_ptr<BaseActionCSP>& manager:_managers) {
// 			if (i == 0 && Config::instance().useMinHMaxActionValueSelector()) { // We initialize the value selector only once
// 				manager->init_value_selector(&bookkeeping);
// 			}
			manager->process(graph);
		}
		
		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (!graph.hasNovelTuples()) return -1;
		
		
		graph.advance(); // Integrates the novel tuples into the graph as a new layer.
		LPT_EDEBUG("heuristic", "New RPG Layer: " << graph);
		
		long h = computeHeuristic(graph);
		if (h > -1) return h;
	}
}

long GecodeCRPG::computeHeuristic(const RPGIndex& graph) const {
	return support::compute_rpg_cost(_tuple_index, graph, *_goal_handler);
}

GecodeCHMax::GecodeCHMax(const Problem& problem, const fs::Formula* goal_formula, const fs::Formula* state_constraints, std::vector<std::shared_ptr<BaseActionCSP>>&& managers, ExtensionHandler extension_handler) :
	GecodeCRPG(problem, goal_formula, state_constraints, std::move(managers), extension_handler) {}
		
long GecodeCHMax::computeHeuristic(const RPGIndex& graph) const {
	return support::compute_hmax_cost(_tuple_index, graph, *_goal_handler);
}

} } // namespaces

