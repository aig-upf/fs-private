
#pragma once

#include <vector>
#include <algorithm>
#include <queue>

#include <state.hxx>
#include <core_problem.hxx>
#include <core_changeset.hxx>
#include <utils/utils.hxx>
#include <action_manager.hxx>

namespace aptk { namespace core {

class RPGraph {
protected:
	const Problem& _problem;
	const State& _seed;
	const std::vector<ChangesetPtr>& _changesets;
	
	std::set<Fact> processed;
	std::queue<const FactSet*> pending;

	std::vector<std::set<BoundActionIdx>> perLayerSupporters;
	
	unsigned numLayers;
	
public:
	
	/**
	 * @param seed The original, non-relaxed state.
 	 * @param changesets An (ordered) stack of changesets representing the planning graph
	 *                  (top element represents last layer).
	 */
	RPGraph(const Problem& problem, const State& seed, const std::vector<ChangesetPtr>& changesets) :
		_problem(problem), _seed(seed), _changesets(changesets),
		processed(),
		pending(),
		perLayerSupporters(changesets.size()),
		numLayers(changesets.size())
	{}


	/**
	 * Builds the relaxed plan from the layered planning graph, whose layers are encoded in `changesets`,
	 * starting from the goal causes, which are given in `causes`.
	 * 
	 * @param causes The facts that allowed the planning graph to reach a goal state.
	 */
	float computeRelaxedPlanCost(const FactSet& causes) {
		
		pending.push(&causes);
		
		while (!pending.empty()) {
			const FactSet* pendingCauses = pending.front(); pending.pop();
			processCauses(pendingCauses);
		}
		
		// Build the relaxed plan by flattening the supporters at each layer.
		ActionPlan plan;
		for (const auto& supporters:perLayerSupporters) {
			plan.insert(plan.end(), supporters.cbegin(), supporters.cend());
		}

		// A check - curently it makes no difference, the plan sizes are always equal
// 		std::set<BoundActionIdx> uniquifiedPlan(plan.begin(), plan.end());
// 		if (plan.size() != uniquifiedPlan.size()) {
// 			std::cout << "RP heuristic deviation of " << plan.size() - uniquifiedPlan.size() << std::endl;
// 		}
// Note that computing the relaxed heuristic by using some form of local consistency might yield plans that are not correct for the relaxation
// 		assert(_checkRPisCorrect(_seed, plan));
// 		std::cout << "Relaxed plan found, cost=" << plan.size() << std::endl;
// 		Utils::printPlan(plan, _problem, std::cout);
		
		return (float) plan.size();
	}
	
	
	void processCauses(const FactSet* causes) {
		for(auto fact:*causes) {
			processCauses(fact);
		}
	}
	
	
	void processCauses(const Fact& fact) {
		if (_seed.contains(fact)) return; // The fact was already on the seed state, thus has empty support.
		if (processed.find(fact) != processed.end()) return; // The fact has already been justfied
		
		// We simply look for the first changeset containing the fact and process its achievers.
		for (unsigned i = 0; i < numLayers; ++i) {
			const ChangesetPtr changeset = _changesets[i];
			const auto achieverAndCauses = changeset->getAchieverAndCauses(fact);
			ActionIdx achiever = achieverAndCauses.first;
			
			if (achiever != CoreAction::INVALID_ACTION) { 
				perLayerSupporters[i].insert(achiever);
				const FactSet& actionCauses = changeset->getCauses(achiever);
				pending.push(&actionCauses); // push the action causes
				pending.push(achieverAndCauses.second.get()); // and push the specific fact extra causes
				processed.insert(fact);
				return;
			}
		}
		throw std::runtime_error("This point should never be reached"); // The achiever should have been found when we reach this point
	}
	

protected:
	/**
	 * Checks that the given relaxed plan is correct, i.e. that it achieves a goal state
	 * in the delete-free relaxation of the problem.
	 */
	bool _checkRPisCorrect(const State& s0, ActionPlan& relaxedPlan) {
		return ActionManager::checkPlanSuccessful(_problem, relaxedPlan, *(RelaxedState::relax(s0)));
	}

};



} } // namespaces
