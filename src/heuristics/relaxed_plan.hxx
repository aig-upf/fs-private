
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
#include "rpg.hxx"

namespace aptk { namespace core {

template < typename SearchModel >
class RelaxedPlanHeuristic : public Heuristic<State> {
public:

	RelaxedPlanHeuristic( const SearchModel& problem ) :
		Heuristic<State>( problem ),
		_problem(problem.getTask()),
		_goal_manager(*_problem.getGoalEvaluator(), _problem.getGoalConstraints())
	{}

	virtual ~RelaxedPlanHeuristic() {}
	
	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	float evaluate(const State& s) {
		
		const ProblemConstraint::vctr& stateConstraints = _problem.getConstraints();
		const ActionList& actions = _problem.getAllActions();
		
		RelaxedState::ptr s1, s0 = RelaxedState::relax(s);
		RelaxedActionSetManager manager(&s, stateConstraints);
		
		std::vector<Changeset::ptr> changesets;
		
		if (_problem.isGoal(s)) return 0; // The seed state is a goal
		
		#ifdef FS0_DEBUG
		std::cout << std::endl << "RP Graph computation from seed state: " << std::endl << s << std::endl << "****************************************" << std::endl;;
		#endif
		
		// The main loop - at each iteration we build an additional RPG layer, until no new atoms are achieved (i.e. the changeset is empty),
		// or we get to a goal graph layer.
		while(true) {
			Changeset::ptr changeset(new Changeset(s0));
			
			for (unsigned idx = 0; idx < actions.size(); ++idx) {
				auto& action = *actions[idx];
				
				// We compute the projection of the current relaxed state to the variables relevant to the action
				DomainSet projection = manager.projectValues(*s0, action);
				
				auto res = manager.isApplicable(action, projection);
				if (res.first) { // The action is applicable in the current RPG layer
					changeset->setCurrentAction(idx, res.second);  // We record the applicability causes
					manager.computeChangeset(action, projection, *changeset);
				}
			}
			
			
			#ifdef FS0_DEBUG
			std::cout << "Changeset size: " << changeset->size() << std::endl;
			// print_changesets(changesets);
			// changeset->printEffects(std::cout);
			#endif

			// If there is no novel fact in the changeset, we reached a fixpoint, thus there is no solution.
			if (changeset->size() == 0) return std::numeric_limits<float>::infinity();
			
			s1 = std::make_shared<RelaxedState>(*s0, *changeset); // Copy everything into a new state & apply the changeset to it.
			#ifdef FS0_DEBUG
			std::cout << std::endl << "New RPG Layer: " << *s1 << std::endl; // std::cout << "Changeset: " << *changeset << std::endl << std::endl;
			#endif
			changesets.push_back(changeset);
			
			float h = computeHeuristic(s, s1, changesets);
			if (h > -1) return h;
			s0 = s1;
		}
	}
	
	virtual float computeHeuristic(const State& seed, const RelaxedState::ptr& s1, const std::vector<Changeset::ptr>& changesets) {
		FactSetPtr causes = std::make_shared<FactSet>();
		if (_goal_manager.isGoal(seed, *s1, causes)) {
			RPGraph rpg = RPGraph(_problem, seed, changesets);
			auto cost = rpg.computeRelaxedPlanCost(causes);
			return cost;
		} else {
			return -1;
		}
	}
	
	void print_changesets(const std::vector<Changeset::ptr>& changesets) {
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
