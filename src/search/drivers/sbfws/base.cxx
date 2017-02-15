
#include <search/drivers/sbfws/base.hxx>
#include <search/novelty/fs_novelty.hxx>
#include <utils/config.hxx>
#include <problem.hxx>

namespace fs0 { namespace bfws {

SBFWSConfig::SBFWSConfig(const Config& config) :
	search_width(config.getOption<int>("width.search", 2)),
	simulation_width(config.getOption<int>("width.simulation", 1)),
	mark_negative_propositions(config.getOption<bool>("simulation.neg_prop", false)),
	complete_simulation(config.getOption<bool>("simulation.complete", true))
{
	std::string rs = config.getOption<std::string>("bfws.rs");
	if  (rs == "aptk_hff") relevant_set_type = RelevantSetType::APTK_HFF;
	else if  (rs == "macro") relevant_set_type = RelevantSetType::Macro;
	else if  (rs == "sim") relevant_set_type = RelevantSetType::Sim;
	else if  (rs == "none") relevant_set_type = RelevantSetType::None;
	else throw std::runtime_error("Unknown option value \"bfws.rs\"=" + rs);
	
	
	if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
		evaluator_t = NoveltyEvaluatorType::Adaptive;
	} else {
		evaluator_t = NoveltyEvaluatorType::Generic;
	}
}



template<>
FSBinaryNoveltyEvaluatorI* create_novelty_evaluator(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType evaluator_t, unsigned max_width, bool persistent) {
	
	if (evaluator_t == SBFWSConfig::NoveltyEvaluatorType::Adaptive) {
		const AtomIndex& index = problem.get_tuple_index();
		
		if (persistent) {
			auto evaluator = FSAtomBinaryNoveltyEvaluatorPersistent::create(index, true, max_width);
			if (evaluator) {
// 				LPT_INFO("cout", "NOVELTY EVALUATION: Using a specialized FS Atom Novelty Evaluator WITH PERSISTENCE");
				return evaluator;
			}			
		} else {
			auto evaluator = FSAtomBinaryNoveltyEvaluator::create(index, true, max_width);
			if (evaluator) {
// 				LPT_INFO("cout", "NOVELTY EVALUATION: Using a specialized FS Atom Novelty Evaluator");
				return evaluator;
			}
		}
	}
	
// 	LPT_INFO("cout", "NOVELTY EVALUATION: Using a binary novelty evaluator");
	return new FSGenericBinaryNoveltyEvaluator(max_width);
}

template<>
FSMultivaluedNoveltyEvaluatorI* create_novelty_evaluator(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType evaluator_t, unsigned max_width, bool persistent) {
	
	/*
	 * TODO - IMPLEMENT THIS FOR MULTIVALUED TYPES
	if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
		const AtomIndex& index = problem.get_tuple_index();
		auto evaluator = FSAtomBinaryNoveltyEvaluator::create(index, true, max_width);
		if (evaluator) {
			LPT_INFO("cout", "Using a specialized FS Atom Novelty Evaluator");
			return evaluator;
		}
	}
	*/
	
	LPT_INFO("cout", "NOVELTY EVALUATION: Using a generic multivalued novelty evaluator");
	return new FSGenericMultivaluedNoveltyEvaluator(max_width);
}
} } // namespaces
