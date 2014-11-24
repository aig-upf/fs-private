
#pragma once

#include <vector>
#include <algorithm>
#include <queue>

#include <aptk/heuristic.hxx>
#include <state.hxx>
#include <core_problem.hxx>
#include <core_changeset.hxx>
#include <utils/utils.hxx>
#include <action_manager.hxx>
#include <constraints/csp_goal_manager.hxx>
#include "rp_graph_procs.hxx"

namespace aptk { namespace core {

template < typename SearchModel >
class RelaxedPlanHeuristic : public Heuristic<State> {
public:

	RelaxedPlanHeuristic( const SearchModel& problem ) :
		Heuristic<State>( problem ),
		_problem(problem.getTask()),
		_goal_manager(*_problem.getGoalEvaluator(), Problem::getCurrentProblem()->getConstraints())
	{}

	virtual ~RelaxedPlanHeuristic() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& s) {
		
		RelaxedState::ptr s1, s0 = RelaxedState::relax(s);
		std::unique_ptr<RelaxedActionSetManager> manager(new RelaxedActionSetManager(&s, *s0));
		
		std::vector<ChangesetPtr> changesets;
		
		if (_problem.isGoal(s)) return 0; // The seed state is a goal
		
// 		std::cout << std::endl << "RP Graph computation from seed state: " << std::endl << s << std::endl;
// 		std::cout << "*************************************************" << std::endl;
		while(true) {
			ChangesetPtr changeset(new Changeset(s0));
			
			for (const auto act: _problem.computeBoundApplicableActions(&s, *s0)) {
				const auto& justifiedAction = act.second;
				changeset->setJustifiedAction(act.first, justifiedAction);
				manager->computeChangeset(*justifiedAction, *changeset);
			}
			
//  			std::cout << "Changeset size: " << changeset->size() << std::endl;
// 			changeset->printEffects(std::cout);

			// If there is no novel fact in the changeset, we reached a fixpoint, thus there is no solution.
			if (changeset->size() == 0) return std::numeric_limits<float>::infinity();
			
			s1 = std::make_shared<RelaxedState>(*s0, *changeset); // Copy everything into a new state & apply the changeset to it.
// 			std::cout << std::endl << "New Relaxed Graph Layer: " << *s1 << std::endl;
// 			std::cout << "Changeset: " << *changeset << std::endl << std::endl;
			changesets.push_back(changeset);
			
			float h = computeHeuristic(s, s1, changesets);
			if (h > -1) return h;
			
			s0 = s1;
		}
	}
	
	virtual float computeHeuristic(const State& seed, const RelaxedState::ptr& s1, const std::vector<ChangesetPtr>& changesets) {
		FactSet causes;
		if (_goal_manager.isGoal(seed, *s1, causes)) {
//  				std::cout << "Original State: " << s << std::endl;
//  				print_changesets(changesets);
			RPGraph rpc = RPGraph(_problem, seed, changesets);
			auto cost = rpc.computeRelaxedPlanCost(causes);
//  				std::cout << "Heuristic value:" << cost << std::endl;
			return cost;
		} else {
			return -1;
		}
	}
	
	void print_changesets(const std::vector<ChangesetPtr>& changesets) {
		for (unsigned i = 0; i < changesets.size(); ++i) {
			std::cout << "Layer #" << i << " changeset: " << std::endl;
			std::cout << *changesets[i] << std::endl;
		}
	}
	
	//! Proxy to circumvent the unusual virtual method signature
	virtual void eval(const State& s, float& h_val) { h_val = evaluate(s); }
	
	//! So far just act as a proxy, we do not compute the preferred operations yet.
	virtual void eval( const State& s, float& h_val,  std::vector<Action_Idx>& pref_ops ) { eval(s, h_val); }
	
protected:
	const Problem& _problem;
	const CSPGoalManager _goal_manager;
};

} } // namespaces
