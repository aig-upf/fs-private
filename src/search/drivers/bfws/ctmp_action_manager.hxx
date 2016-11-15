
#pragma once
#include <applicability/action_managers.hxx>


namespace fs0 { class Config; }

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class CTMPActionManager : public SmartActionManager {
public:
	using ApplicableSet = GroundApplicableSet;
	
	CTMPActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints, const TupleIndex& tuple_idx, const BasicApplicabilityAnalyzer* analyzer);
	~CTMPActionManager() = default;
	CTMPActionManager(const CTMPActionManager&) = default;
	
	GroundApplicableSet applicable(const State& state) const;
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool applicable(const State& state, const GroundAction& action) const;
	
protected:
// 	std::vector<ActionIdx> compute_whitelist(const State& state) const override;
	bool check_constraints(unsigned action_id, const State& state) const override;
};

} // namespaces
