
#pragma once

#include <vector>
#include <algorithm>
#include <queue>

#include <state.hxx>
#include <core_problem.hxx>
#include <heuristics/changeset.hxx>
#include <utils/utils.hxx>
#include <utils/printers.hxx>
#include <action_manager.hxx>

namespace aptk { namespace core {

/**
 * A proper Relaxed Planning Graph. This class is used to perform the plan extraction from
 * an already existing set of changesets that do actually represent the RPG.
 */
class RPGraph {
protected:
	const Problem& _problem;
	const State& _seed;
	const std::vector<Changeset::ptr>& _changesets;
	
	std::set<Fact> processed;
	std::queue<std::shared_ptr<const Fact::vctr>> pending;
	
	typedef std::vector<std::set<ActionIdx>> LayeredSupporters;

public:
	
	/**
	 * @param seed The original, non-relaxed state.
 	 * @param changesets An (ordered) stack of changesets representing the planning graph
	 *                  (top element represents last layer).
	 */
	RPGraph(const Problem& problem, const State& seed, const Changeset::vptr& changesets) :
		_problem(problem), _seed(seed), _changesets(changesets),
		processed(),
		pending()
	{}


	/**
	 * Builds the relaxed plan from the layered planning graph, whose layers are encoded in `changesets`,
	 * starting from the goal causes, which are given in `causes`.
	 * 
	 * @param causes The facts that allowed the planning graph to reach a goal state.
	 */
	float computeRelaxedPlanCost(const Fact::vctrp causes) {
		
		pending.push(causes);
		LayeredSupporters perLayerSupporters(_changesets.size());  // Make room for the supporters in each layer
		
		while (!pending.empty()) {
			std::shared_ptr<const Fact::vctr> pendingCauses = pending.front(); pending.pop();
			processCauses(pendingCauses, perLayerSupporters);
		}
		
		// Build the relaxed plan by flattening the supporters at each layer.
		ActionPlan plan;
		for (const auto& supporters:perLayerSupporters) {
			plan.insert(plan.end(), supporters.cbegin(), supporters.cend());
		}

		// A check - curently it makes no difference, the plan sizes are always equal
// 		std::set<ActionIdx> uniquifiedPlan(plan.begin(), plan.end());
// 		if (plan.size() != uniquifiedPlan.size()) {
// 			std::cout << "RP heuristic deviation of " << plan.size() - uniquifiedPlan.size() << std::endl;
// 		}
// Note that computing the relaxed heuristic by using some form of local consistency might yield plans that are not correct for the relaxation
// 		assert(_checkRPisCorrect(_seed, plan));

		#ifdef FS0_DEBUG
		std::cout << "Relaxed plan found, cost=" << plan.size() << std::endl;
		Printers::printPlan(plan, _problem, std::cout);
		#endif

		
		return (float) plan.size();
	}
	
	//! Returns only those facts that were not in the seed state.
	Fact::vctrp pruneSeedSupporters(const FactSetPtr& causes) const {
		return pruneSeedSupporters(causes, _seed);
	}
	
	//! Returns only those facts that were not in the given seed state.
	static Fact::vctrp pruneSeedSupporters(const FactSetPtr& causes, const State& seed) {
		Fact::vctrp nonSeed = std::make_shared<Fact::vctr>();
		for(const auto fact:*causes) {
			if (!seed.contains(fact)) {
				nonSeed->push_back(fact);
			}
		}
		return nonSeed;
	}
	
protected:
	
	//! A set of atoms is processed by processing each of the atoms one by one
	void processCauses(std::shared_ptr<const Fact::vctr> causes, LayeredSupporters& perLayerSupporters) {
		for(auto fact:*causes) {
			processCauses(fact, perLayerSupporters);
		}
	}
	
	//! Process a single atom by seeking its supports left-to-right in the RPG and enqueuing them to be further processed
	void processCauses(const Fact& fact, LayeredSupporters& perLayerSupporters) {
		if (_seed.contains(fact)) return; // The fact was already on the seed state, thus has empty support.
		if (processed.find(fact) != processed.end()) return; // The fact has already been justfied
		
		// We simply look for the first changeset containing the fact and process its achievers.
		for (unsigned i = 0; i < _changesets.size(); ++i) {
			const Changeset::ptr changeset = _changesets[i];
			const auto achieverAndCauses = changeset->getAchieverAndCauses(fact);
			ActionIdx achiever = std::get<0>(achieverAndCauses);
			
			if (achiever != CoreAction::INVALID_ACTION) { 
				perLayerSupporters[i].insert(achiever);
				pending.push(changeset->getCauses(achiever)); // push the action causes themselves
 				pending.push(std::get<1>(achieverAndCauses)); // and push the specific extra causes that were relevant to turn the fact true.
				processed.insert(fact);
				return;
			}
		}
		throw std::runtime_error("This point should never be reached"); // The achiever should have been found when we reach this point
	}

	/**
	 * Checks that the given relaxed plan is correct, i.e. that it achieves a goal state
	 * in the delete-free relaxation of the problem.
	 */
	bool _checkRPisCorrect(const State& s0, ActionPlan& relaxedPlan) {
		return ActionManager::checkRelaxedPlanSuccessful(_problem, relaxedPlan, s0);
	}

};



} } // namespaces
