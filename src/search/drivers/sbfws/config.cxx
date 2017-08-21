
#include "config.hxx"
#include <utils/config.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 { namespace bfws {

SBFWSConfig::SBFWSConfig(const Config& config) :
	search_width(config.getOption<int>("width.search", 2)),
	simulation_width(config.getOption<int>("width.simulation", 1)),
	mark_negative_propositions(config.getOption<bool>("simulation.neg_prop", false)),
	complete_simulation(config.getOption<bool>("simulation.complete", true)),
	using_feature_set(config.getOption<bool>("bfws.using_feature_set", false))
{
	LPT_INFO("search", "width.search=" << search_width );
	LPT_INFO("search", "width.simulation=" << simulation_width );
	LPT_INFO("search", "bfws.using_feature_set=" << using_feature_set);
	std::string rs = config.getOption<std::string>("bfws.rs");
	if  (rs == "sim") relevant_set_type = RelevantSetType::Sim;
	else if  (rs == "l0" ) relevant_set_type = RelevantSetType::L0;
	else if (rs == "g0" ) relevant_set_type = RelevantSetType::G0;
	else if  (rs == "none") relevant_set_type = RelevantSetType::None;
	else throw std::runtime_error("Unknown option value \"bfws.rs\"=" + rs);
	LPT_INFO("search", "R set definition: " << rs );

	if (config.getOption<std::string>("evaluator_t", "") == "adaptive") {
		evaluator_t = NoveltyEvaluatorType::Adaptive;
		LPT_INFO("search", "Using ADAPTIVE novelty evaluator" );
	} else {
		evaluator_t = NoveltyEvaluatorType::Generic;
		LPT_INFO("search", "Using GENERIC novelty evaluator" );
	}

	std::string rcomp = config.getOption<std::string>("bfws.rcomp", "seed");
	if  (rcomp == "seed") {
		r_computation = RComputation::Seed;
		LPT_INFO("search", "R Computed at the initial state only");
	}
	else if  (rcomp == "decrease") {
		r_computation = RComputation::GDecr;
		LPT_INFO("search", "R Computed when #g(s) decreases");
	}
	else throw std::runtime_error("Unknown option value \"bfws.rcomp\"=" + rcomp);

}


} } // namespaces
