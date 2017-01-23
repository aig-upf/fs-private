
#include <search/drivers/sbfws/base.hxx>
#include <utils/config.hxx>

namespace fs0 { namespace bfws {

SBFWSConfig::SBFWSConfig(const Config& config) :
	search_width(config.getOption<int>("width.search")),
	simulation_width(config.getOption<int>("width.simulation")),
	mark_negative_propositions(config.getOption<bool>("relevance.neg_prop"))
{
	relevant_set_type = RelevantSetType::Sim;
	std::string rs = config.getOption<std::string>("bfws.rs");
	if (rs == "hff") relevant_set_type = RelevantSetType::HFF;
	else if  (rs == "aptk_hff") relevant_set_type = RelevantSetType::APTK_HFF;
}

} } // namespaces
