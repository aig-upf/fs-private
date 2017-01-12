
#include <search/drivers/sbfws/base.hxx>
#include <utils/config.hxx>

namespace fs0 { namespace bfws {

SBFWSConfig::SBFWSConfig(const Config& config) :
	search_width(config.getOption<int>("width.search")),
	simulation_width(config.getOption<int>("width.simulation")),
	mark_negative_propositions(config.getOption<bool>("relevance.neg_prop"))
{}

} } // namespaces
