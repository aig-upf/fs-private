
#pragma once

#include <vector>
#include <algorithm>
#include <queue>

#include <state.hxx>
#include <problem.hxx>
#include <heuristics/changeset.hxx>
#include <utils/utils.hxx>
#include <utils/printers.hxx>
#include <action_manager.hxx>

namespace fs0 {

/**
 * A proper Relaxed Planning Graph. This class is used to perform the plan extraction from
 * an already existing set of changesets that do actually represent the RPG.
 */
class RPGraph {
protected:
	const State& _seed;
	const std::vector<Changeset::ptr>& _changesets;
	
	typedef std::vector<std::set<ActionIdx>> LayeredSupporters;
	LayeredSupporters perLayerSupporters;
	
	std::set<Fact> processed;
	std::queue<Fact> pending;

public:
	
	/**
	 * @param seed The original, non-relaxed state.
 	 * @param changesets An (ordered) stack of changesets representing the planning graph
	 *                  (top element represents last layer).
	 */
	RPGraph(const State& seed, const Changeset::vptr& changesets) :
		_seed(seed), _changesets(changesets), perLayerSupporters(changesets.size()),
		processed(), pending()
	{}


	/**
	 * Builds the relaxed plan from the layered planning graph, whose layers are encoded in `changesets`,
	 * starting from the goal causes, which are given in `causes`.
	 * 
	 * @param causes The atoms that allowed the planning graph to reach a goal state.
	 */
	float computeRelaxedPlanCost(const Fact::vctr& causes) {
		
		enqueueAtoms(causes);
		
		while (!pending.empty()) {
			const Fact& atom = pending.front();
			processAtom(atom);
			pending.pop();
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
		// assert(ActionManager::checkRelaxedPlanSuccessful(*Problem::getCurrentProblem(), plan, _seed));
		#ifdef FS0_DEBUG
		std::cout << "Relaxed plan found, cost=" << plan.size() << std::endl;
		Printers::printPlan(plan, *Problem::getCurrentProblem(), std::cout);
		#endif

		
		return (float) plan.size();
	}
	
	//! Returns only those atoms that were not in the given seed state.
	static Fact::vctrp pruneSeedSupporters(const Fact::vctr& causes, const State& seed) {
		Fact::vctrp notInSeed = std::make_shared<Fact::vctr>();
		for(const auto& atom:causes) {
			if (!seed.contains(atom)) {
				notInSeed->push_back(atom);
			}
		}
		return notInSeed;
	}
	
protected:
	
	inline void enqueueAtoms(const Fact::vctr& atoms) {
		for(auto& atom:atoms) pending.push(atom);
	}

	//! Process a single atom by seeking its supports left-to-right in the RPG and enqueuing them to be further processed
	void processAtom(const Fact& atom) {
		if (_seed.contains(atom)) return; // The atom was already on the seed state, thus has empty support.
		if (processed.find(atom) != processed.end()) return; // The atom has already been justfied
		
		// We simply look for the first changeset containing the atom and process its achievers.
		for (unsigned i = 0; i < _changesets.size(); ++i) {
			const Changeset::ptr changeset = _changesets[i];
			const auto& achieverAndCauses = changeset->getAchieverAndCauses(atom);
			ActionIdx achiever = std::get<0>(achieverAndCauses);
			
			if (achiever != Action::INVALID) { 
				perLayerSupporters[i].insert(achiever);
				enqueueAtoms(*(changeset->getCauses(achiever))); // push the action causes themselves
 				enqueueAtoms(*(std::get<1>(achieverAndCauses))); // and push the specific extra causes that were relevant to turn the atom true.
				processed.insert(atom);
				return;
			}
		}
		throw std::runtime_error("This point should never be reached"); // The achiever should have been found when we reach this point
	}

};



} // namespaces
