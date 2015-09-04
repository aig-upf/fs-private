
#pragma once

#include <fs0_types.hxx>
#include <state_model.hxx>
#include <utils/logging.hxx>
#include <heuristics/relaxed_plan/constrained_relaxed_plan_heuristic.hxx>
#include <heuristics/novelty/novelty_from_preconditions.hxx>
#include "novelty_evaluator.hxx"

namespace fs0 { class Problem; class Config; }

namespace fs0 { namespace engines {
	
template <typename SearchNode>
class NoveltyHeuristic : public NoveltyEvaluator<SearchNode> {
	typedef NoveltyEvaluator<SearchNode> BaseClass;
	
public:
	NoveltyHeuristic(const FS0StateModel& model) : BaseClass(model) {}

	float evaluate(const State& state) {
		auto novelty = BaseClass::novelty(state);
		if (novelty > BaseClass::novelty_bound()) return std::numeric_limits<float>::infinity();
		
		

		assert(0); // This needs to be properly implemented and clearly distinguished from gbfs(f)
		
// 		num_unsat = heuristic.evaluate_num_unsat_goals( state );
// 		SearchStatistics::instance().min_num_goals_sat = std::min( num_unsat, SearchStatistics::instance().min_num_goals_sat);
// 		if ( parent != nullptr && num_unsat < parent->num_unsat ) {
			//std::cout << "Reward!" << std::endl;
			//print(std::cout);
			//std::cout << std::endl;
// 		}
		/* NOT USED as this is the heuristic function for gbfs(f)
		h = heuristic.evaluate_reachability( state );
		unsigned ha = 2;
		if ( parent != nullptr && h < parent->h ) ha = 1;
		f = 2 * (novelty - 1) + ha;
		*/		
		
	}
};

} } // namespaces
