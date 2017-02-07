
#include <search/drivers/sbfws/sgbfs.hxx>
#include <search/utils.hxx>
#include <models/simple_state_model.hxx>
#include <aptk2/tools/logging.hxx>
#include <heuristics/novelty/features.hxx>

namespace fs0 { namespace bfws {


template<>
FSBinaryNoveltyEvaluatorI* create_novelty_evaluator(const Problem& problem, const Config& config, unsigned max_width) {
	
	if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
		const AtomIndex& index = problem.get_tuple_index();
		auto evaluator = FSAtomBinaryNoveltyEvaluator::create(index, true);
		if (evaluator) {
			LPT_INFO("cout", "Using a specialized FS Atom Novelty Evaluator");
			return evaluator;
		}
	}
	
	LPT_INFO("cout", "Using a Binary Novelty Evaluator");
	return new FSGenericBinaryNoveltyEvaluator(max_width);
}

//! Factory method
template <typename StateModelT, typename FeatureSetT, typename NoveltyEvaluatorT>
std::unique_ptr<LazyBFWS<StateModelT, FeatureSetT, NoveltyEvaluatorT>>
create(const Config& config, SBFWSConfig& conf, const StateModelT& model, BFWSStats& stats) {
	
	// Engine types
	using EngineT = LazyBFWS<StateModelT, FeatureSetT, NoveltyEvaluatorT>;
	
	auto search_evaluator = create_novelty_evaluator<NoveltyEvaluatorT>(model.getTask(), config, conf.search_width);
	auto simulation_evaluator = create_novelty_evaluator<NoveltyEvaluatorT>(model.getTask(), config, conf.simulation_width);
	
	return std::unique_ptr<EngineT>(new EngineT(model, search_evaluator, simulation_evaluator, stats, config, conf));
}

template <>
ExitCode
LazyBFWSDriver<SimpleStateModel>::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	return do_search(drivers::GroundingSetup::fully_ground_simple_model(problem), config, out_dir, start_time);
}


template <typename StateModelT>
ExitCode
LazyBFWSDriver<StateModelT>::do_search(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time) {

	if (true) { // TODO - IF THE STATE CONTAINS ONLY BINARY VARIABLES
		return do_search2<FSBinaryNoveltyEvaluatorI, lapkt::novelty::StraightBinaryFeatureSetEvaluator<StateT>>(model, config, out_dir, start_time);
		
	} else {
		return do_search2<FSMultivaluedNoveltyEvaluatorI, lapkt::novelty::StraightMultivaluedFeatureSetEvaluator<StateT>>(model, config, out_dir, start_time);
	}
}

/*
template <typename StateModelT>
template <typename NoveltyEvaluatorT>
ExitCode
LazyBFWSDriver<StateModelT>::do_search1(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time) {

	if (true) {
		return do_search2<NoveltyEvaluatorT, lapkt::novelty::StraightBinaryFeatureSetEvaluator<StateT>>(model, config, out_dir, start_time);
	} else {
		
	}
}
*/

template <typename StateModelT>
template <typename NoveltyEvaluatorT, typename FeatureSetT>
ExitCode
LazyBFWSDriver<StateModelT>::do_search2(const StateModelT& model, const Config& config, const std::string& out_dir, float start_time) {
	SBFWSConfig bfws_config(config);
	
	
	auto engine = create<StateModelT, FeatureSetT, NoveltyEvaluatorT>(config, bfws_config, model, _stats);
	
	
	LPT_INFO("cout", "Simulated BFWS Configuration:");
// 	LPT_INFO("cout", "\tMaximum search novelty: " << bfws_config.search_width);
// 	LPT_INFO("cout", "\tMaximum simulation novelty: " << bfws_config.simulation_width);
	LPT_INFO("cout", "\tMark as relevant negative propositional atoms?: " << bfws_config.mark_negative_propositions);
// 	LPT_INFO("cout", "\tFeature extraction: " << feature_configuration);
	return drivers::Utils::do_search(*engine, model, out_dir, start_time, _stats);
}





} } // namespaces
