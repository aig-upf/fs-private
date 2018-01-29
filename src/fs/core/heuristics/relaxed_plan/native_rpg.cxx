
#include <limits>

#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/heuristics/relaxed_plan/native_rpg.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/heuristics/relaxed_plan/relaxed_plan.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/constraints/gecode/lifted_plan_extractor.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/problem.hxx>

namespace fs0 { namespace gecode {

NativeRPG::NativeRPG(const Problem& problem, const fs::Formula* goal_formula, const std::vector<const fs::Formula*>& state_constraints, std::vector<HandlerPT>&& managers, ExtensionHandler extension_handler) :
	_problem(problem),
	_info(ProblemInfo::getInstance()),
	_tuple_index(problem.get_tuple_index()),
	_managers(std::move(managers)),
	_extension_handler(std::move(extension_handler)),
    _goal_checker(goal_formula, _tuple_index, true)
{
	LPT_INFO("heuristic", "NativeRPG heuristic initialized");
	if (_managers.empty()) {
		LPT_INFO("cout", "*** WARNING - Heuristic initialized with no applicable action ***");
	}
}


//! The actual evaluation of the heuristic value for any given non-relaxed state s.
long NativeRPG::evaluate(const State& seed, std::vector<Atom>& relevant) {

	if (_problem.getGoalSatManager().satisfied(seed)) return 0; // The seed state is a goal

	LPT_EDEBUG("heuristic", std::endl << "Computing RPG from seed state: " << std::endl << seed << std::endl << "****************************************");

	RPGIndex graph(seed, _tuple_index, _extension_handler);

	while (true) {

		// Build a new layer of the RPG.
		for (const HandlerPT& manager:_managers) {
			manager->process(graph);
		}

		// If there is no novel fact in the rpg, we reached a fixpoint, thus there is no solution.
		if (!graph.hasNovelTuples()) return -1;

		graph.advance(); // Integrates the novel tuples into the graph as a new layer.
		LPT_EDEBUG("heuristic", "New RPG Layer: " << graph);

		long h = computeHeuristic(graph, relevant);
		if (h > -1) return h;
	}
}

long NativeRPG::computeHeuristic(const RPGIndex& graph, std::vector<Atom>& relevant) {

    std::vector<AtomIdx> support;
    if (!_goal_checker.check_reachable(graph, support)) return -1; // The goal doesn't hold


    LiftedPlanExtractor extractor(graph, _tuple_index);
    return extractor.computeRelaxedPlanCost(support, relevant);
}


} } // namespaces
