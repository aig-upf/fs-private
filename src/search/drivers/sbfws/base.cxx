
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
	if  (rs == "sim") relevant_set_type = RelevantSetType::Sim;
	else if  (rs == "none") relevant_set_type = RelevantSetType::None;
	else throw std::runtime_error("Unknown option value \"bfws.rs\"=" + rs);
	
	
	if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
		evaluator_t = NoveltyEvaluatorType::Adaptive;
	} else {
		evaluator_t = NoveltyEvaluatorType::Generic;
	}
	
	std::string rcomp = config.getOption<std::string>("bfws.rcomp", "seed");
	if  (rcomp == "seed") r_computation = RComputation::Seed;
	else if  (rcomp == "decrease") r_computation = RComputation::GDecr;
	else throw std::runtime_error("Unknown option value \"bfws.rcomp\"=" + rcomp);
}



template<>
FSBinaryNoveltyEvaluatorI* create_novelty_evaluator(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType evaluator_t, unsigned max_width) {
	const AtomIndex& index = problem.get_tuple_index();	
	const Config& config = Config::instance(); // TODO - Remove the singleton use and inject the config here by other means
	bool ignore_neg_literals = config.getOption<bool>("ignore_neg_literals", true);
	
	if (evaluator_t == SBFWSConfig::NoveltyEvaluatorType::Adaptive) {
		auto evaluator = FSAtomBinaryNoveltyEvaluator::create(index, ignore_neg_literals, max_width);
		if (evaluator) {
			LPT_INFO("cout", "NOVELTY EVALUATION: Using a specialized FS Atom Novelty Evaluator");
			return evaluator;
		}
	}
	
 	LPT_INFO("cout", "NOVELTY EVALUATION: Using a binary novelty evaluator");
	return new FSGenericBinaryNoveltyEvaluator(max_width);
}

template<>
FSMultivaluedNoveltyEvaluatorI* create_novelty_evaluator(const Problem& problem, SBFWSConfig::NoveltyEvaluatorType evaluator_t, unsigned max_width) {
	const Config& config = Config::instance(); // TODO - Remove the singleton use and inject the config here by other means
	if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
		// TODO - IMPLEMENT THIS FOR MULTIVALUED TYPES
		/*
		const AtomIndex& index = problem.get_tuple_index();
		auto evaluator = FSAtomBinaryNoveltyEvaluator::create(index, true, max_width);
		if (evaluator) {
			LPT_INFO("cout", "Using a specialized FS Atom Novelty Evaluator");
			return evaluator;
		}
		*/
	}
	
	
	LPT_INFO("cout", "NOVELTY EVALUATION: Using a generic multivalued novelty evaluator");
	return new FSGenericMultivaluedNoveltyEvaluator(max_width);
}
} } // namespaces
