
#pragma once

#include <applicability/action_managers.hxx>

namespace fs0 {


//! An applicability analyzer based on a gecode CSP model
class GecodeApplicabilityAnalyzer : public BasicApplicabilityAnalyzer {
public:
	GecodeApplicabilityAnalyzer(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_idx) : 
		BasicApplicabilityAnalyzer(actions, tuple_idx) {}
	
	~GecodeApplicabilityAnalyzer() = default;
	
	void build() override;

};


} // namespaces

