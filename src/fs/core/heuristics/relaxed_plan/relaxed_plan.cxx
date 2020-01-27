
#include <fs/core/heuristics/relaxed_plan/relaxed_plan.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/constraints/gecode/lifted_plan_extractor.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 { namespace gecode { namespace support {

long compute_rpg_cost(const AtomIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler, std::vector<Atom>& relevant) {
	long cost = -1;
	if (GecodeSpace* csp = goal_handler.instantiate(graph)) {
		if (csp->propagate()) { // ATM we only take into account full goal resolution
			LPT_EDEBUG("heuristic", "Goal formula CSP is consistent: " << *csp);
			std::vector<AtomIdx> causes;
			if (goal_handler.compute_support(csp, causes)) {
				LiftedPlanExtractor extractor(graph, tuple_index);
				cost = extractor.computeRelaxedPlanCost(causes, relevant);
			}
		}
		delete csp;
	}
	return cost;
}

long compute_rpg_cost(const AtomIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler) {
	std::vector<Atom> _;
	return compute_rpg_cost(tuple_index, graph, goal_handler, _);
}

long compute_hmax_cost(const AtomIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler) {
	long cost = -1;
	if (GecodeSpace* csp = goal_handler.instantiate(graph)) {
		if (csp->propagate() && goal_handler.is_satisfiable(csp)) { // ATM we only take into account full goal resolution
			LPT_EDEBUG("heuristic", "Found solution of Goal formula CSPn");
			cost = graph.getCurrentLayerIdx();
		}
		delete csp;
	}
	return cost;
}





} } } // namespaces
