
#include <heuristics/relaxed_plan/relaxed_plan.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <constraints/gecode/handlers/formula_csp.hxx>
#include <constraints/gecode/lifted_plan_extractor.hxx>
#include <aptk2/tools/logging.hxx>

namespace fs0 { namespace gecode { namespace support {

long compute_rpg_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler) {
	long cost = -1;
	if (GecodeCSP* csp = goal_handler.instantiate(graph)) {
		if (csp->checkConsistency()) { // ATM we only take into account full goal resolution
			LPT_EDEBUG("heuristic", "Goal formula CSP is consistent: " << *csp);
			std::vector<TupleIdx> causes;
			if (goal_handler.compute_support(csp, causes)) {
				LiftedPlanExtractor extractor(graph, tuple_index);
				cost = extractor.computeRelaxedPlanCost(causes);
			}
		}
		delete csp;
	}
	return cost;
}

long compute_hmax_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const FormulaCSP& goal_handler) {
	long cost = -1;
	if (GecodeCSP* csp = goal_handler.instantiate(graph)) {
		if (csp->checkConsistency() && goal_handler.is_satisfiable(csp)) { // ATM we only take into account full goal resolution
			LPT_EDEBUG("heuristic", "Found solution of Goal formula CSPn");
			cost = graph.getCurrentLayerIdx();
		}
		delete csp;
	}
	return cost;
}





} } } // namespaces
