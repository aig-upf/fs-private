
#include <search/drivers/bfws/iw_novelty_evaluator.hxx>
#include <aptk2/tools/logging.hxx>

namespace fs0 { namespace bfws {


std::vector<ValuesTuple::ValueIndex>
IWNoveltyEvaluator::compute_valuation(const State& state) const {
// 	LPT_INFO("novelty-evaluations", "Evaluating state " << state);
	std::vector<ValuesTuple::ValueIndex> values;
	
	values.reserve(_features.size());
	for (const auto& feature:_features) {
		values.push_back(feature->evaluate(state));
		// LPT_INFO("novelty-evaluations", "\t" << _featureMap.feature(k) << ": " << values[k]);
	}

	return values;
// 	LPT_DEBUG("heuristic", "Feature evaluation: " << std::endl << print::feature_set(varnames, values));
}



IWNoveltyEvaluator::IWNoveltyEvaluator(unsigned novelty_bound, const FeatureSet& features) :
	Base(novelty_bound),
	_features(features)
{}






} } // namespaces
