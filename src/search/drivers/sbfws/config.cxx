
#include "config.hxx"
#include <utils/config.hxx>

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


} } // namespaces
