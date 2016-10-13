
#include <search/drivers/bfws/bfws.hxx>
#include <search/events.hxx>
#include <search/utils.hxx>
#include <actions/ground_action_iterator.hxx>
#include <search/drivers/smart_effect_driver.hxx>
#include <search/drivers/setups.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace drivers {


template <typename StateModelT, typename ActionT>
typename BFWSSubdriverF0<StateModelT, ActionT>::Engine
BFWSSubdriverF0<StateModelT, ActionT>::create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const StateModelT& model) {
	
	using EvaluatorT = EvaluationObserver<NodeT, HeuristicT>;
	using StatsT = StatsObserver<NodeT>;

	_heuristic = std::unique_ptr<HeuristicT>(new HeuristicT(model, bfws_config._max_width, feature_configuration));
	
	_handlers.push_back(std::unique_ptr<StatsT>(new StatsT(_stats)));
	_handlers.push_back(std::unique_ptr<EvaluatorT>(new EvaluatorT(*_heuristic, config.getNodeEvaluationType())));
	
	auto engine = new lapkt::StlBestFirstSearch<NodeT, HeuristicT, StateModelT>(model, *_heuristic);
	lapkt::events::subscribe(*engine, _handlers);

	return Engine(engine);
}


template <typename NodeT,
          typename HeuristicT,
          typename NodeCompareT,
          typename HeuristicEnsembleT,
          typename RawEngineT,
          typename Engine
>
typename BFWS1H1WSubdriver<NodeT, HeuristicT, NodeCompareT, HeuristicEnsembleT, RawEngineT, Engine>::EngineT
BFWS1H1WSubdriver<NodeT, HeuristicT, NodeCompareT, HeuristicEnsembleT, RawEngineT, Engine>::create(const Config& config, BFWSConfig& bfws_config, const NoveltyFeaturesConfiguration& feature_configuration, const GroundStateModel& model) {
	
	using EvaluatorT = EvaluationObserver<NodeT, HeuristicEnsembleT>;
	using StatsT = StatsObserver<NodeT>;

	auto base_heuristic = std::unique_ptr<HeuristicT>(SmartEffectDriver::configure_heuristic(model.getTask(), config));
	_heuristic = std::unique_ptr<HeuristicEnsembleT>(new HeuristicEnsembleT(model, bfws_config._max_width, feature_configuration, std::move(base_heuristic)));
	
	_handlers.push_back(std::unique_ptr<StatsT>(new StatsT(_stats)));
	_handlers.push_back(std::unique_ptr<EvaluatorT>(new EvaluatorT(*_heuristic, config.getNodeEvaluationType())));
	
	auto engine = new RawEngineT(model, *_heuristic);
	lapkt::events::subscribe(*engine, _handlers);

	return Engine(engine);
}


//! Helper
template <typename StateModelT, typename SubdriverT>
void do_search(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time) {
	BFWSConfig bfws_config(config);
	SubdriverT subdriver;
	NoveltyFeaturesConfiguration feature_configuration(config);
	auto engine = subdriver.create(config, bfws_config, feature_configuration, model);
	LPT_INFO("cout", "BFWS Configuration:");
	LPT_INFO("cout", "\tBFWS Type: " << bfws_config._type);
	LPT_INFO("cout", "\tMax novelty: " << bfws_config._max_width);
	LPT_INFO("cout", "\tFeature extraction: " << feature_configuration);
	Utils::do_search(*engine, model, out_dir, start_time, subdriver.getStats());
}



template <>
void 
BFWSDriver<GroundStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	switch(BFWSConfig(config)._type) {
		case BFWSConfig::Type::F0: do_search<GroundStateModel, BFWS_F0_GROUND>(GroundingSetup::fully_ground_model(problem), config, out_dir, start_time); break;
		case BFWSConfig::Type::F1: do_search<GroundStateModel, BFWS_F1>(GroundingSetup::ground_search_lifted_heuristic(problem), config, out_dir, start_time); break;
		case BFWSConfig::Type::F2: do_search<GroundStateModel, BFWS_F2>(GroundingSetup::ground_search_lifted_heuristic(problem), config, out_dir, start_time); break;
		case BFWSConfig::Type::F5: do_search<GroundStateModel, BFWS_F5>(GroundingSetup::ground_search_lifted_heuristic(problem), config, out_dir, start_time); break;
		default: throw std::runtime_error("Invalid BFWS type");
	}
}


template <>
void 
BFWSDriver<LiftedStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	switch(BFWSConfig(config)._type) {
 		case BFWSConfig::Type::F0: do_search<LiftedStateModel, BFWS_F0_LIFTED>(GroundingSetup::fully_lifted_model(problem), config, out_dir, start_time); break;
// 		case BFWSConfig::Type::F1: do_search<LiftedStateModel, BFWS_F1>(GroundingSetup::ground_search_lifted_heuristic(problem), config, out_dir, start_time); break;
// 		case BFWSConfig::Type::F2: do_search<LiftedStateModel, BFWS_F2>(GroundingSetup::ground_search_lifted_heuristic(problem), config, out_dir, start_time); break;
		default: throw std::runtime_error("Invalid BFWS type");
	}
}


} } // namespaces
