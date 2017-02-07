
#include <search/drivers/sbfws/base.hxx>
#include <utils/config.hxx>

namespace fs0 { namespace bfws {

SBFWSConfig::SBFWSConfig(const Config& config) :
	search_width(config.getOption<int>("width.search", 1)),
	simulation_width(config.getOption<int>("width.simulation", 1)),
	mark_negative_propositions(config.getOption<bool>("simulation.neg_prop", false)),
	complete_simulation(config.getOption<bool>("simulation.complete", true))
{
	std::string rs = config.getOption<std::string>("bfws.rs");
	if (rs == "hff") relevant_set_type = RelevantSetType::HFF;
	else if  (rs == "aptk_hff") relevant_set_type = RelevantSetType::APTK_HFF;
	else if  (rs == "macro") relevant_set_type = RelevantSetType::Macro;
	else if  (rs == "sim") relevant_set_type = RelevantSetType::Sim;
	else throw std::runtime_error("Unknown option value \"bfws.rs\"=" + rs);
}

} } // namespaces
