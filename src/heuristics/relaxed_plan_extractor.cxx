
#include <heuristics/relaxed_plan_extractor.hxx>
#include <utils/config.hxx>

namespace fs0 {

BaseRelaxedPlanExtractor* RelaxedPlanExtractorFactory::create(const State& seed, const RPGData& data) {
		const Config& config = Config::instance();
		if (config.getRPGExtractionType() == Config::RPGExtractionType::Propositional) {
			return new PropositionalRelaxedPlanExtractor(seed, data);
		} else {
			return new SupportedRelaxedPlanExtractor(seed, data);
		}
		
}
} // namespaces

