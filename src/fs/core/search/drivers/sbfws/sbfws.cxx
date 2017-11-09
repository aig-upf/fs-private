
#include <lapkt/tools/logging.hxx>

#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/search/drivers/sbfws/features/features.hxx>
#include <fs/core/search/drivers/sbfws/sbfws.hxx>
#include <fs/core/search/drivers/sbfws/mv_iw_run.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/models/simple_state_model.hxx>

#include <fs/core/heuristics/novelty/features.hxx>

namespace fs0 { namespace bfws {


//! Factory method
template <	typename StateModelT, typename FeatureEvaluatorT, typename NoveltyEvaluatorT,
			template <class N, class S, class NE, class FS> class SimulatorT,
			template <class S, class A> class SimNodeT>
std::unique_ptr<SBFWS<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT, SimulatorT, SimNodeT>>
create(FeatureEvaluatorT&& featureset, SBFWSConfig& config, const StateModelT& model, BFWSStats& stats) {
	using EngineT = SBFWS<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT, SimulatorT, SimNodeT>;
	return std::unique_ptr<EngineT>(new EngineT(model, std::move(featureset), stats, config));
}

template <>
ExitCode
SBFWSDriver<SimpleStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	return do_search(drivers::GroundingSetup::fully_ground_simple_model(problem), config, out_dir, start_time);
}


template <>
ExitCode
SBFWSDriver<LiftedStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	return do_search(drivers::GroundingSetup::fully_lifted_model(problem), config, out_dir, start_time);
}

template <typename StateModelT>
ExitCode
SBFWSDriver<StateModelT>::do_search(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time) {
	const StateAtomIndexer& indexer = model.getTask().getStateAtomIndexer();

	if (config.getOption<bool>("width.force_generic_evaluator", false)) {
		FeatureSelector<StateT> selector(ProblemInfo::getInstance());

		LPT_INFO("search", "FEATURE EVALUATION: Forced to use GenericFeatureSetEvaluator");
		using FeatureEvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<StateT>;
		return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, selector.select(), config, out_dir, start_time);
	}
	if (config.getOption<bool>("bfws.extra_features", false)) {
		FeatureSelector<StateT> selector(ProblemInfo::getInstance());

		if (selector.has_extra_features()) {
			LPT_INFO("search", "FEATURE EVALUATION: Extra Features were found!  Using a GenericFeatureSetEvaluator");
			using FeatureEvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<StateT>;
			return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, selector.select(), config, out_dir, start_time);
		}
	}

	if (indexer.is_fully_binary()) { // The state is fully binary
		LPT_INFO("search", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<bool>");
		using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<bool>;
		return do_search1<BoolNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time);

	}
	/*
	else if (indexer.is_fully_multivalued()) { // The state is fully multivalued
		LPT_INFO("search", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<object_id>");
		using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<int>;
		return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time);

	}*/
	else { // We have a hybrid state and cannot thus apply optimizations
		LPT_INFO("search", "FEATURE EVALUATION: Using a generic IntegerFeatureEvaluator");
		using FeatureEvaluatorT = bfws::IntegerFeatureEvaluator;
		return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time);
	}
}


template <typename StateModelT>
template <typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
ExitCode
SBFWSDriver<StateModelT>::do_search1(const StateModelT& model, FeatureEvaluatorT&& featureset, const Config& config, const std::string& out_dir, float start_time) {
	SBFWSConfig bfws_config(config);

	if ( bfws_config.using_feature_set) {
		auto engine = create<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT, MultiValuedIWRun, MultiValuedIWRunNode>(std::move(featureset), bfws_config, model, _stats);
		return drivers::Utils::SearchExecution<StateModelT>(model).do_search(*engine, out_dir, start_time, _stats);
	}

	auto engine = create<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT, IWRun, IWRunNode>(std::move(featureset), bfws_config, model, _stats);
	return drivers::Utils::SearchExecution<StateModelT>(model).do_search(*engine, out_dir, start_time, _stats);


}



} } // namespaces
