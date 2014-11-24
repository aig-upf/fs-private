
#ifndef __APTK_CORE_RELAXED_PLAN_GRAPH__
#define __APTK_CORE_RELAXED_PLAN_GRAPH__

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
	const std::vector<State::ptr>& _states;

	std::set<Fact> processedFacts;
	std::set<NegatedFact> processedNFacts;
	
	std::queue<Formula::cptr> pending;
	std::vector<std::set<BoundActionIdx>> perLayerSupporters;
	
	unsigned numLayers;
	
public:
	RPGraph( const Problem& problem, const State& seed, const std::vector<ChangesetPtr>& changesets, const std::vector<State::ptr>& states) :
		_problem(problem), _seed(seed), _changesets(changesets), _states(states),
		processedFacts(), processedNFacts(), pending(), perLayerSupporters(changesets.size()),
		numLayers(changesets.size())
	{
		assert(changesets.size() == states.size());
	}


	float computeRelaxedPlanCost(const PlainConjunctiveFact::cptr& goalCause) {
		processCauses(goalCause);
		
		while (!pending.empty()) {
			const Formula::cptr cause = pending.front(); pending.pop();
			processCauses(cause);
		}
		
		// Build the relaxed plan
		ActionPlan plan;
		for (const auto& supporters:perLayerSupporters) {
			plan.insert(plan.end(), supporters.cbegin(), supporters.cend());
		}

		// A check - curently it makes no difference, the plan sizes are always equal
// 		std::set<BoundActionIdx> uniquifiedPlan(plan.begin(), plan.end());
// 		if (plan.size() != uniquifiedPlan.size()) {
// 			std::cout << "RP heuristic deviation of " << plan.size() - uniquifiedPlan.size() << std::endl;
// 		}

		assert(_checkRPisCorrect(_seed, plan));
		//std::cout << "Relaxed Plan: " << plan.size() << std::endl;
		//Utils::printPlan(plan, _problem, std::cout);
		
		return (float) plan.size();
	}
	
	
	void processCauses(const Formula::cptr& formula) {
		// Dispatch the formula to a different method depending on its actual type
		// This is less clean than a double-dispatch-based design, but introduces less
		// coupling.
		
		if (auto cast = std::dynamic_pointer_cast<const PlainConjunctiveFact>(formula)) {
			_processCauses(cast);
			return;
		}

		// Note: order matters, this cast must go before the more generic cast to Fact.
		if (auto cast = std::dynamic_pointer_cast<const NegatedFact>(formula)) {
			_processCauses(cast);
			return;
		}
		
		if (auto cast = std::dynamic_pointer_cast<const Fact>(formula)) {
			_processCauses(cast);
			return;
		}
		
		assert(false); // Need to implement this
		throw NonImplementedException();
	}
	
	void _processCauses(const PlainConjunctiveFact::cptr& cause) {
		for(auto fact:cause->elements) {
			processCauses(fact);
		}
	}

/*	
	void processCauses(const PlainDisjunctiveFact::cptr& cause) {
		for(auto fact:cause->elements) {
			processCauses(fact);
		}
	}
*/	
	
	void _processCauses(const Fact::cptr& fact) {
		if (_seed.contains(*fact)) return; // The fact was already on the seed state, thus has empty support.
		if (processedFacts.find(*fact) != processedFacts.end()) return; // The fact has already been justfied
		
		// We simply look for the first changeset containing the fact and process its achievers.
		for (unsigned i = 0; i < numLayers; ++i) {
			const ChangesetPtr changeset = _changesets[i];
			BoundActionIdx achieverActionIdx = changeset->getAchiever(*fact);
			
			if (achieverActionIdx != CoreAction::INVALID_ACTION) { 
				perLayerSupporters[i].insert(achieverActionIdx);
				pending.push(_problem.getAction(achieverActionIdx)->getApplicabilityFormula());
				processedFacts.insert(*fact);
				return;
			}
		}
		assert(false); // The achiever should have been found when we reach this point
	}
	
	void _processCauses(const NegatedFact::cptr& fact) {
		if (!_seed.contains(*fact)) return; // If the positive fact is not on the seed state, it has empty support
		if (processedNFacts.find(*fact) != processedNFacts.end()) return; // The fact has already been justfied
		
		// TODO - Performance improvement: We could check here if there is any fact in either the set of processed facts
		// TODO - or the set of pending facts that fulfills this negation.
		
		
		// We look for the first changeset containing a fact with same variable and different value, and process its achievers.
		for (unsigned i = 0; i < numLayers; ++i) {
			const ChangesetPtr changeset = _changesets[i];
			const State::ptr state = _states[i];
			
			const ObjectSet& values = state->getValues(fact->_variable);
			
			assert(values.size() > 0); // The state must have at least one value for the given state variable
			
			// If layer "i" only contains the _positive_ fact, thus has no possible supporter for our negated fact.
			if (values.size() == 1 && values.find(fact->_value) != values.end()) {
				continue;
			}
			
			// We select one arbitrary fact that fulfills our inequality.
			// TODO - Note that this is a possibly suboptimal choice that could be improved
			ObjectIdx unequalValue = 0; // To get rid of the warning, we'll change it anyhow.
			bool found = false;
			for(const auto& val:values) {
				if (val != fact->_value) {
					unequalValue = val;
					found = true;
					break;
				}
			}
			if (!found) assert(false); // To disable the "variable 'found' set but not used" warning
			
			Fact positive(fact->_variable, unequalValue);
			BoundActionIdx achieverActionIdx = changeset->getAchiever(positive);
			assert(achieverActionIdx != CoreAction::INVALID_ACTION); // The achiever must necessarily be in the same layer.
			
			perLayerSupporters[i].insert(achieverActionIdx);
			pending.push(_problem.getAction(achieverActionIdx)->getApplicabilityFormula());
			
			processedNFacts.insert(*fact);
			processedFacts.insert(positive);
			
			break;
		}
	}
	

protected:
	/**
	 * Checks that the given relaxed plan is correct, i.e. that it achieves a goal state
	 * in the delete-free relaxation of the problem.
	 */
	bool _checkRPisCorrect(const State& s0, ActionPlan& relaxedPlan) {
		// We relax the state so that the check is implicitly on a relaxed version of the problem.
		return ActionManager::checkPlanSuccessful(_problem, relaxedPlan, *(State::relax(s0)));
	}

};
























} } // namespaces

#endif
