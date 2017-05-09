
#include <lapkt/tools/logging.hxx>

#include "base.hxx"
#include "features/features.hxx"
#include <search/drivers/sbfws/sbfws.hxx>
#include <search/utils.hxx>
#include <models/simple_state_model.hxx>

#include <heuristics/novelty/features.hxx>

namespace fs0 { namespace bfws {


//! Factory method
template <typename StateModelT, typename FeatureEvaluatorT, typename NoveltyEvaluatorT>
std::unique_ptr<SBFWS<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>>
create(const Config& config, FeatureEvaluatorT&& featureset, SBFWSConfig& conf, const StateModelT& model, BFWSStats& stats) {
	using EngineT = SBFWS<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>;
	return std::unique_ptr<EngineT>(new EngineT(model, std::move(featureset), stats, config, conf));
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
	if (config.getOption<bool>("bfws.extra_features", false)) {
		FeatureSelector<StateT> selector(ProblemInfo::getInstance());

		if (selector.has_extra_features() || selector.projecting_away_vars() ) {
			LPT_INFO("cout", "FEATURE EVALUATION: Extra Features were found!  Using a GenericFeatureSetEvaluator");
			using FeatureEvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<StateT>;
			return do_search1<FSMultivaluedNoveltyEvaluatorI, FeatureEvaluatorT>(model, selector.select(), config, out_dir, start_time);
		}
	}

	if (indexer.is_fully_binary()) { // The state is fully binary
		LPT_INFO("cout", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<bin>");
		using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<bool>;
		return do_search1<FSBinaryNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time);

	} else if (indexer.is_fully_multivalued()) { // The state is fully multivalued
		LPT_INFO("cout", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<int>");
		using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<int>;
		return do_search1<FSMultivaluedNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time);

	} else { // We have a hybrid state and cannot thus apply optimizations
		LPT_INFO("cout", "FEATURE EVALUATION: Using a generic StraightHybridFeatureSetEvaluator");
		using FeatureEvaluatorT = lapkt::novelty::StraightHybridFeatureSetEvaluator;
		return do_search1<FSMultivaluedNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, out_dir, start_time);
	}
}


template <typename StateModelT>
template <typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
ExitCode
SBFWSDriver<StateModelT>::do_search1(const StateModelT& model, FeatureEvaluatorT&& featureset, const Config& config, const std::string& out_dir, float start_time) {
	SBFWSConfig bfws_config(config);

	auto engine = create<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>(config, std::move(featureset), bfws_config, model, _stats);

	return drivers::Utils::do_search(*engine, model, out_dir, start_time, _stats);
}



} } // namespaces
