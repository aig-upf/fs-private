
#include <lapkt/tools/logging.hxx>

#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/search/drivers/sbfws/features/features.hxx>
#include <fs/core/search/drivers/sbfws/sbfws.hxx>
#include <fs/core/search/utils.hxx>

namespace fs0::bfws {


//! Factory method
template <typename StateModelT, typename FeatureEvaluatorT, typename NoveltyEvaluatorT>
std::unique_ptr<SBFWS<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>>
create(FeatureEvaluatorT&& featureset, SBFWSConfig& config, const StateModelT& model, BFWSStats& stats) {
    using EngineT = SBFWS<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>;
    return std::unique_ptr<EngineT>(new EngineT(model, std::forward<FeatureEvaluatorT>(featureset), stats, config));
}

template <>
ExitCode
SBFWSDriver<SimpleStateModel>::search(Problem& problem, const Config& config, const drivers::EngineOptions& options, float start_time) {
    return do_search(drivers::GroundingSetup::fully_ground_simple_model(problem), config, options, start_time);
}


template <>
ExitCode
SBFWSDriver<CSPLiftedStateModel>::search(Problem& problem, const Config& config, const drivers::EngineOptions& options, float start_time) {
    return do_search(drivers::GroundingSetup::csp_lifted_model(problem), config, options, start_time);
}

template <>
ExitCode
SBFWSDriver<SDDLiftedStateModel>::search(Problem& problem, const Config& config, const drivers::EngineOptions& options, float start_time) {
    return do_search(drivers::GroundingSetup::sdd_lifted_model(problem), config, options, start_time);
}

template <typename StateModelT>
ExitCode
SBFWSDriver<StateModelT>::do_search(const StateModelT& model, const Config& config, const drivers::EngineOptions& options, float start_time) {
    const StateAtomIndexer& indexer = model.getTask().getStateAtomIndexer();

    if (config.getOption<bool>("width.force_generic_evaluator", false)) {
        FeatureSelector<StateT> selector(ProblemInfo::getInstance());

        LPT_INFO("search", "FEATURE EVALUATION: Forced to use GenericFeatureSetEvaluator");
        using FeatureEvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<StateT>;
        return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, selector.select(), config, options, start_time);
    }
    if (config.getOption<bool>("bfws.extra_features", false)) {
        FeatureSelector<StateT> selector(ProblemInfo::getInstance());

        if (selector.has_extra_features()) {
            LPT_INFO("search", "FEATURE EVALUATION: Extra Features were found! Using a GenericFeatureSetEvaluator");
            using FeatureEvaluatorT = lapkt::novelty::GenericFeatureSetEvaluator<StateT>;
            return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, selector.select(), config, options, start_time);
        }
    }

    if (indexer.is_fully_binary()) { // The state is fully binary
        LPT_INFO("search", "FEATURE EVALUATION: Using the specialized StraightFeatureSetEvaluator<bool>");
        using FeatureEvaluatorT = lapkt::novelty::StraightFeatureSetEvaluator<bool>;
        return do_search1<BoolNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, options, start_time);

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
        return do_search1<IntNoveltyEvaluatorI, FeatureEvaluatorT>(model, FeatureEvaluatorT(), config, options, start_time);
    }
}


template <typename StateModelT>
template <typename NoveltyEvaluatorT, typename FeatureEvaluatorT>
ExitCode
SBFWSDriver<StateModelT>::do_search1(const StateModelT& model, FeatureEvaluatorT&& featureset, const Config& config, const drivers::EngineOptions& options, float start_time) {
    SBFWSConfig bfws_config(config);

    bool actionless = model.getTask().getPartiallyGroundedActions().empty() &&
            model.getTask().getGroundActions().empty();

    auto engine = create<StateModelT, FeatureEvaluatorT, NoveltyEvaluatorT>(std::forward<FeatureEvaluatorT>(featureset), bfws_config, model, _stats);

    return drivers::Utils::SearchExecution<StateModelT>(model).do_search(*engine, options, start_time, _stats, actionless);
}

} // namespaces
