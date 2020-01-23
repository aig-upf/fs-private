

#include <lapkt/novelty/features.hxx>
#include <fs/core/search/drivers/iterated_width.hxx>
#include <fs/core/search/novelty/fs_novelty.hxx>
#include <fs/core/search/utils.hxx>

#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/search/drivers/sbfws/features/features.hxx>


namespace fs0::drivers {


template <typename StateModelT, typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
std::unique_ptr<FS0IWAlgorithm<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>>
create(const Config& config, FeatureEvaluatorT&& featureset, const StateModelT& model, SearchStats& stats) {
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;

	using EngineT = FS0IWAlgorithm<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>;
	using EnginePT = std::unique_ptr<EngineT>;

	unsigned max_novelty = config.getOption<int>("width.max");
//	assert(0); // TO REIMPLEMENT
// 	auto evaluator = fs0::bfws::create_novelty_evaluator<NoveltyEvaluatorT>(model.getTask(), fs0::bfws::SBFWSConfig::NoveltyEvaluatorType::Adaptive, max_novelty);
//	auto evaluator = nullptr;
	bfws::NoveltyFactory<FeatureValueT> factory(model.getTask(), bfws::SBFWSConfig::NoveltyEvaluatorType::Generic, true, max_novelty);
	return EnginePT(new EngineT(model, 1, max_novelty, std::move(featureset), factory.create_evaluator(max_novelty), stats));
}




template <typename StateModelT, typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
ExitCode
do_search1(const StateModelT& model, FeatureEvaluatorT&& featureset, const Config& config, const EngineOptions& options, float start_time, SearchStats& stats) {
	auto engine = create<StateModelT, NoveltyEvaluatorT, FeatureEvaluatorT>(config, std::move(featureset), model, stats);
	return Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, stats);

}




template <typename StateModelT>
ExitCode
do_search(const StateModelT& model, const Config& config, const EngineOptions& options, float start_time, SearchStats& stats) {
	const StateAtomIndexer& indexer = model.getTask().getStateAtomIndexer();
	if (config.getOption<bool>("bfws.extra_features", false)) {
		fs0::bfws::FeatureSelector<State> selector(ProblemInfo::getInstance());

		if (selector.has_extra_features()) {
			LPT_INFO("search", "FEATURE EVALUATION: Extra Features were found!  Using a GenericFeatureSetEvaluator");
			using FeatureEvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<State>;
			return do_search1<StateModelT, bfws::IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, selector.select(), config, options, start_time, stats);
		}
	}

	if (indexer.is_fully_binary()) { // The state is fully binary
		LPT_INFO("search", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<bool>");
		using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<bool>;
		return do_search1<StateModelT, bfws::BoolNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, options, start_time, stats);

	}
// 	else if (indexer.is_fully_multivalued()) { // The state is fully multivalued
// 		LPT_INFO("search", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<object_id>");
// 		using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<object_id>;
// 		return do_search1<StateModelT, bfws::IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time, stats);
//
// 	}
	else { // We have a hybrid state and cannot thus apply optimizations
		LPT_INFO("search", "FEATURE EVALUATION: Using a generic IntegerFeatureEvaluator");
		using FeatureEvaluatorT = bfws::IntegerFeatureEvaluator;
		return do_search1<StateModelT, bfws::IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, options, start_time, stats);
	}
}


//////////////////////////////////////////////////////////////////////


/*
template <typename StateModelT>
typename IteratedWidthDriver<StateModelT>::EnginePT
create(const Config& config, const StateModelT& model) {
	using Engine = FS0IWAlgorithm<StateModelT>;
	using EnginePT = std::unique_ptr<Engine>;
	unsigned max_novelty = config.getOption<int>("width.max");
	return EnginePT(new Engine(model, 1, max_novelty, _stats));
}
*/

template <>
ExitCode
IteratedWidthDriver<GroundStateModel>::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	auto model = GroundingSetup::fully_ground_model(problem);
	return do_search(model, config, options, start_time, _stats);
// 	auto engine = create(config, model);
// 	return Utils::do_search(*engine, model, out_dir, start_time, _stats);
}

template <>
ExitCode
IteratedWidthDriver<CSPLiftedStateModel>::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	auto model = GroundingSetup::csp_lifted_model(problem);
	return do_search(model, config, options, start_time, _stats);
// 	auto engine = create(config, model);
// 	return Utils::do_search(*engine, model, out_dir, start_time, _stats);
}


} // namespaces
