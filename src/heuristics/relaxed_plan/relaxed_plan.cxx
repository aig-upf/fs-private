
#include <heuristics/relaxed_plan/relaxed_plan.hxx>
#include "rpg_index.hxx"
#include <constraints/gecode/handlers/lifted_formula_handler.hxx>
#include <constraints/gecode/lifted_plan_extractor.hxx>
#include <utils/logging.hxx>

namespace fs0 { namespace gecode { namespace support {

long compute_rpg_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const LiftedFormulaHandler& goal_handler) {
	long cost = -1;
	if (SimpleCSP* csp = goal_handler.instantiate(graph)) {
		if (csp->checkConsistency()) { // ATM we only take into account full goal resolution
			FFDEBUG("heuristic", "Goal formula CSP is consistent: " << *csp);
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

long compute_hmax_cost(const TupleIndex& tuple_index, const RPGIndex& graph, const LiftedFormulaHandler& goal_handler) {
	long cost = -1;
	if (SimpleCSP* csp = goal_handler.instantiate(graph)) {
		if (csp->checkConsistency() && goal_handler.is_satisfiable(csp)) { // ATM we only take into account full goal resolution
			FFDEBUG("heuristic", "Found solution of Goal formula CSPn");
			cost = graph.getCurrentLayerIdx();
		}
		delete csp;
	}
	return cost;
}





} } } // namespaces
