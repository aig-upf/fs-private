
#pragma once

#include <fs_types.hxx>

//#include <heuristics/novelty/features.hxx>
#include <unordered_set>
//#include <utils/external.hxx>


namespace fs0 { class Config; class Problem; class BasicApplicabilityAnalyzer; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

using OffendingSet = std::unordered_set<fs0::ObjectIdx>;


namespace fs0 { namespace drivers {


class EBFWSPreprocessor {
public:
	static std::vector<OffendingSet> preprocess(const Problem& problem, const Config& config, BasicApplicabilityAnalyzer* analyzer);
	
};

} } // namespaces



