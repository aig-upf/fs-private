
#include <algorithm>

#include <constraints/gecode/lifted_plan_extractor.hxx>

#include <state.hxx>
#include <problem.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/utils.hxx>
#include <utils/printers/printers.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>
#include <utils/tuple_index.hxx>
#include <actions/action_id.hxx>

namespace fs0 { namespace gecode {

	
LiftedPlanExtractor::LiftedPlanExtractor(const State& seed, const RPGIndex& graph, const TupleIndex& tuple_index) :
	_seed(seed), _graph(graph), processed(), pending(), perLayerSupporters(graph.getNumLayers()), _tuple_index(tuple_index)
{}


long LiftedPlanExtractor::computeRelaxedPlanCost(const std::vector<TupleIdx>& tuples) {
	enqueueTuples(tuples);
	
	while (!pending.empty()) {
		TupleIdx tuple = pending.front();
		processTuple(tuple);
		pending.pop();
	}
	
	return buildRelaxedPlan();
}


void LiftedPlanExtractor::processTuple(TupleIdx tuple) {
	if (_seed.contains(_tuple_index.to_atom(tuple))) return; // The atom was already on the seed state, thus has empty support.
	if (processed.find(tuple) != processed.end()) return; // The atom has already been processed
	
	const RPGIndex::TupleSupport& support = _graph.getTupleSupport(tuple);
	
	const ActionID* action_id = std::get<1>(support);
	assert(action_id);
// 	std::cout << "Inserting: " << *action_id << " on layer #" << std::get<0>(support) << ", support size: " << std::get<2>(support).size() << std::endl;
	perLayerSupporters[std::get<0>(support)].insert(action_id);
	enqueueTuples(std::get<2>(support)); // Push the full support of the atom
	processed.insert(tuple); // Tag the atom as processed.
}


long LiftedPlanExtractor::buildRelaxedPlan() {
#ifndef DEBUG
	// In production mode, we simply count the number of actions in the plan, but prefer not to build the actual plan.
	unsigned size = 0;
	for (const auto& supporters:perLayerSupporters) {
		size += supporters.size();
	}
	return (long) size;
#endif

	// In debug mode, we build the relaxed plan by flattening the supporters at each layer, so that we can log the actual plan.
	plan_t plan;
	for (const auto& supporters:perLayerSupporters) {
		plan.insert(plan.end(), supporters.cbegin(), supporters.cend());
	}

	// Note that computing the relaxed heuristic by using some form of local consistency might yield plans that are not correct for the relaxation
	// assert(ActionManager::checkRelaxedPlanSuccessful(Problem::getInstance(), plan, _seed));
	FFDEBUG("heuristic" , "Relaxed plan found with length " << plan.size() << std::endl << print::plan(plan));

	return (long) plan.size();
}

} } // namespaces
