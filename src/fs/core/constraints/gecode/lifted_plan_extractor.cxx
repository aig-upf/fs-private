
#include <algorithm>

#include <fs/core/constraints/gecode/lifted_plan_extractor.hxx>

#include <fs/core/state.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/utils//printers/printers.hxx>
#include <fs/core/utils//printers/actions.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 { namespace gecode {

	
LiftedPlanExtractor::LiftedPlanExtractor(const RPGIndex& graph, const AtomIndex& tuple_index) :
	_graph(graph), processed(), pending(), perLayerSupporters(graph.getNumLayers()), _tuple_index(tuple_index)
{}


long LiftedPlanExtractor::computeRelaxedPlanCost(const std::vector<AtomIdx>& goal_support, std::vector<Atom>& relevant) {
	enqueueTuples(goal_support);
	
	while (!pending.empty()) {
		AtomIdx tuple = pending.front();
		processTuple(tuple, relevant);
		pending.pop();
	}
	
	return buildRelaxedPlan();
}

void LiftedPlanExtractor::enqueueTuples(const std::vector<AtomIdx>& tuples) {
	for(const auto& tuple:tuples) {
		pending.push(tuple);
	}
}

void LiftedPlanExtractor::processTuple(AtomIdx tuple, std::vector<Atom>& relevant) {
	const Atom& atom = _tuple_index.to_atom(tuple);
	if (_graph.getSeed().contains(atom)) return; // The atom was already on the seed state, thus has empty support.
	if (processed.find(tuple) != processed.end()) return; // The atom has already been processed
	
	const RPGIndex::TupleSupport& support = _graph.getTupleSupport(tuple);
	
	const ActionID* action_id = std::get<1>(support);
	assert(action_id);
	unsigned layer_idx = std::get<0>(support);
// 	std::cout << "Inserting: " << *action_id << " on layer #" << std::get<0>(support) << ", support size: " << std::get<2>(support).size() << std::endl;
	perLayerSupporters[layer_idx].insert(action_id);
	enqueueTuples(std::get<2>(support)); // Push the full support of the atom
	processed.insert(tuple); // Tag the atom as processed.
	
	// We store all those atoms that have been identified as supports of some action of the relaxed plan
	// and are on the first layer of the RPG
	if (layer_idx == 1) {
		relevant.push_back(atom);
	}
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
	LPT_EDEBUG("relaxed-plan" , "Relaxed plan (" << plan.size() << ") for state: " <<  std::endl << _graph.getSeed() << std::endl << "\t"  << print::plan(plan) << std::endl);
	

	return (long) plan.size();
}

} } // namespaces
