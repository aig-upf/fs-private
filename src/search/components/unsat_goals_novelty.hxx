
#pragma once

#include <fs_types.hxx>
#include <ground_state_model.hxx>
#include <utils/logging.hxx>
#include <heuristics/novelty/fs0_novelty_evaluator.hxx>
#include <heuristics/novelty/novelty_features_configuration.hxx>
#include <heuristics/unsat_goal_atoms/unsat_goal_atoms.hxx>
#include <search/components/base_novelty_component.hxx>

namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace drivers {

//! A novelty evaluator object to be used as an open list acceptor.
//! It accepts a new search node iff its novelty less than or equal to the max novelty bound
template <typename SearchNode>
class UnsatGoalsNoveltyComponent : public BaseNoveltyComponent<SearchNode> {
protected:
	//! We have k+1 novelty evaluators, where k is the number of goal conditions, so that we can evaluate the novelty
	//! of a state with respect to all previous states __with the same number of unsatisfied goals__
	//! Thus, '_novelty_evaluators[i]' gives us the novelty evaluator that contains the data of all
	//! previous states which had 'i' unsatisfied goal conditions
	std::vector<GenericNoveltyEvaluator> _novelty_evaluators;
	
	//! An UnsatisfiedGoalAtomsHeuristic to count the number of unsatisfied goals
	UnsatisfiedGoalAtomsHeuristic _unsat_goal_atoms_heuristic;

public:
	typedef BaseNoveltyComponent<SearchNode> Base;

	UnsatGoalsNoveltyComponent(const GroundStateModel& model, unsigned max_novelty, const NoveltyFeaturesConfiguration& feature_configuration)
		: Base(max_novelty), 
		  _novelty_evaluators(model.getTask().getGoalConditions()->all_atoms().size()+1, GenericNoveltyEvaluator(model.getTask(), max_novelty, feature_configuration)), // We set up k+1 identical evaluators
		  _unsat_goal_atoms_heuristic(model)
	{
		if (!dynamic_cast<const fs::Conjunction*>(model.getTask().getGoalConditions())) {
			throw std::runtime_error("NoveltyComponent available only for goal conjunctions");
		}
	}
	
	~UnsatGoalsNoveltyComponent() {
		for (unsigned j = 0; j < _novelty_evaluators.size(); j++)
			for ( unsigned k = 1; k <= Base::novelty_bound(); k++ ) {
				FINFO("heuristic", "# novelty(s)[#goals=" << j << "]=" << k << " : " << _novelty_evaluators[j].get_num_states(k));
			}
	}

	unsigned evaluate_num_unsat_goals(const State& state) const { return _unsat_goal_atoms_heuristic.evaluate(state); }

	GenericNoveltyEvaluator& evaluator(const State& state) { return _novelty_evaluators[evaluate_num_unsat_goals(state)]; }
};

} } // namespaces
