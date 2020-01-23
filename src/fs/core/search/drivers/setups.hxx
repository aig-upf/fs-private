
#pragma once

#include <fs/core/models/csp_lifted_state_model.hxx>
#include <fs/core/models/sdd_lifted_state_model.hxx>
#include <fs/core/models/ground_state_model.hxx>
#include <fs/core/models/simple_state_model.hxx>

#include <lapkt/tools/events.hxx>
#include <fs/core/search/events.hxx>

namespace fs0 { class Problem; }

namespace fs0 { namespace drivers {

//! A catalog of common setups for grounding actions for both search and heuristic computations.
class GroundingSetup {
public:
	static CSPLiftedStateModel csp_lifted_model(Problem& problem);

    static SDDLiftedStateModel sdd_lifted_model(Problem& problem);
	
	//! A simple model with all grounded actions
	static GroundStateModel fully_ground_model(Problem& problem);
	
	static SimpleStateModel fully_ground_simple_model(Problem& problem);
	
	//! We'll use all the ground actions for the search plus the partially ground actions for the heuristic computations
	static GroundStateModel ground_search_lifted_heuristic(Problem& problem);

};

class EventUtils {
public:
	using HandlerPtr = std::unique_ptr<lapkt::events::EventHandler>;
	
	template <typename NodeT, typename HeuristicT, typename StatsT>
	static void setup_evaluation_observer(const Config& config, HeuristicT& heuristic, StatsT& stats, std::vector<HandlerPtr>& handlers) {
		using EvaluatorT = EvaluationObserver<NodeT, HeuristicT, StatsT>;
		handlers.push_back(std::unique_ptr<EvaluatorT>(new EvaluatorT(heuristic, config.getNodeEvaluationType(), stats)));
	}
	
	template <typename NodeT, typename StatsT>
	static void setup_stats_observer(StatsT& stats, std::vector<HandlerPtr>& handlers, bool verbose = false) {
		using StatsObserverT = StatsObserver<NodeT, StatsT>;
		handlers.push_back(std::unique_ptr<StatsObserverT>(new StatsObserverT(stats, verbose)));
	}
	
	template <typename NodeT>
	static void setup_HA_observer(std::vector<HandlerPtr>& handlers) {
		using HAObserverT = HelpfulObserver<NodeT>;
		handlers.push_back(std::unique_ptr<HAObserverT>(new HAObserverT()));
	}
	
};

} } // namespaces
