
#pragma once

#include <memory>
#include <vector>
#include <constraints/direct/constraint.hxx>

namespace fs0 {

/**
 * A Sum constraint custom propagator. 
 */
class SumConstraint : public DirectConstraint
{
public:
	
	SumConstraint(const VariableIdxVector& scope);
	SumConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~SumConstraint() {}
	
	virtual FilteringType filteringType() const override { return FilteringType::Custom; };
	
	//! Filters from the set of currently loaded projections
	FilteringOutput filter() override;
	
	virtual DirectConstraint* compile(const ProblemInfo& problemInfo) const { return nullptr; }
	
	std::ostream& print(std::ostream& os) const;
};


} // namespaces

