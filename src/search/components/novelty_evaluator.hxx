
#pragma once

#include <fs0_types.hxx>
#include <state_model.hxx>
#include <utils/logging.hxx>
#include <heuristics/novelty/fs0_novelty_evaluator.hxx>
#include <heuristics/unsat_goal_atoms/unsat_goal_atoms.hxx>

namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace engines {

//! A novelty evaluator object to be used as an open list acceptor.
//! It accepts a new search node iff its novelty less than or equal to the max novelty bound
template <typename SearchNode>
class NoveltyEvaluator {
protected:
	const Problem& _problem;

	std::vector<GenericNoveltyEvaluator> _novelty_heuristic;

	unsigned _max_novelty;

	UnsatisfiedGoalAtomsHeuristic _unsat_goal_atoms_heuristic;

public:

	NoveltyEvaluator(const FS0StateModel& model)
		: _problem(model.getTask()), _novelty_heuristic(_problem.getGoalConditions().size() + 1), _max_novelty(0), _unsat_goal_atoms_heuristic(model)
	{}

	~NoveltyEvaluator() {
		for (unsigned j = 0; j < _novelty_heuristic.size(); j++)
			for ( unsigned k = 1; k <= novelty_bound(); k++ ) {
				FINFO("heuristic", "# novelty(s)[#goals=" << j << "]=" << k << " : " << _novelty_heuristic[j].get_num_states(k));
			}
	}

	void setup(int max_novelty, bool useStateVars, bool useGoal, bool useActions) {
		_max_novelty = max_novelty;
		for ( unsigned k = 0; k < _novelty_heuristic.size(); k++ ) {
			_novelty_heuristic[k].set_max_novelty( novelty_bound() );
			_novelty_heuristic[k].selectFeatures( _problem, useStateVars, useGoal, useActions );
		}
	}

	unsigned evaluate_num_unsat_goals(const State& state) const { return _unsat_goal_atoms_heuristic.evaluate(state); }

	inline unsigned novelty_bound() { return _max_novelty; }

	inline unsigned novelty(const State& state) {
		GenericNoveltyEvaluator& evaluator = _novelty_heuristic[evaluate_num_unsat_goals(state)];
		return evaluator.evaluate(state);
	}

	//! Returns false iff we want to prune this node during the search
	bool accept(const SearchNode& n) {
		return novelty(n.state) <= novelty_bound();
	}
};

} } // namespaces
