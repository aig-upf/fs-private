
#include <search/drivers/gbfs_novelty.hxx>
#include <search/events.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace drivers {
	
GBFSNoveltyDriver::Engine
GBFSNoveltyDriver::create(const Config& config, const GroundStateModel& model) {
	
	unsigned max_novelty = config.getOption<int>("width.max_novelty");

	NoveltyFeaturesConfiguration feature_configuration(config);
	_heuristic = std::unique_ptr<NoveltyHeuristic>(new NoveltyHeuristic(model, max_novelty, feature_configuration));
	
	using EvaluatorT = EvaluationObserver<NodeT, NoveltyHeuristic>;
	using StatsT = StatsObserver<NodeT>;
	
	_handlers.push_back(std::unique_ptr<StatsT>(new StatsT(_stats)));
	_handlers.push_back(std::unique_ptr<EvaluatorT>(new EvaluatorT(*_heuristic, config.getNodeEvaluationType())));
	
	auto engine = new lapkt::StlBestFirstSearch<NodeT, NoveltyHeuristic, GroundStateModel>(model, *_heuristic);
	lapkt::events::subscribe(*engine, _handlers);
	
	LPT_INFO("main", "Heuristic options:");
	LPT_INFO("main", "\tMax novelty: " << max_novelty);
	LPT_INFO("main", "\tFeatiue extaction: " << feature_configuration);
	
	return Engine(engine);
}

GroundStateModel
GBFSNoveltyDriver::setup(const Config& config, Problem& problem) const {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem); // By default we ground all actions and return a model with the problem as it is
}

} } // namespaces
