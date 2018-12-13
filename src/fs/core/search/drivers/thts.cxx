

#include <lapkt/novelty/features.hxx>

#include <fs/core/search/drivers/thts.hxx>
#include <fs/core/search/novelty/fs_novelty.hxx>
#include <fs/core/search/utils.hxx>
#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/search/drivers/sbfws/features/features.hxx>
#include <fs/core/search/algorithms/tree_search.hxx>
#include <fs/core/search/drivers/setups.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/models/ground_state_model.hxx>


namespace fs0 { namespace drivers {



template <typename StateModelT, typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
std::unique_ptr<TreeSearch<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>>
create(const Config& config, FeatureEvaluatorT&& featureset, const StateModelT& model) {
	using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;
    using EngineT = TreeSearch<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>;

	// Create the novelty evaluator
	unsigned max_novelty = static_cast<unsigned>(config.getOption<int>("width.max", 2));
	bfws::NoveltyFactory<FeatureValueT> factory(model.getTask(), false, max_novelty);
    auto novelty_evaluator = factory.create_compound_evaluator(max_novelty);
	return std::make_unique<EngineT>(model, max_novelty, std::move(featureset), novelty_evaluator);
}



template <typename StateModelT>
ExitCode
do_search_thts(const StateModelT& model, const Config& config, const EngineOptions& options, float start_time, SearchStats& stats) {
	const StateAtomIndexer& indexer = model.getTask().getStateAtomIndexer();

	assert(indexer.is_fully_binary());
    LPT_INFO("search", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<bool>");
    using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<bool>;
    auto engine = create<GroundStateModel, bfws::BoolNoveltyEvaluatorI, FeatureEvaluatorT>(config, FeatureEvaluatorT(), model);
    return Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, stats);
}



template <>
ExitCode
THTSDriver<GroundStateModel>::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	auto model = GroundingSetup::fully_ground_model(problem);
	return do_search_thts(model, config, options, start_time, _stats);
}



} } // namespaces
