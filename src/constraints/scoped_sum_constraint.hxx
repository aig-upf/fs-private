
#pragma once

#include <memory>
#include <vector>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {


/**
 * A Sum constraint custom propagator. 
 */
class ScopedSumConstraint : public ScopedConstraint
{
public:
	
	ScopedSumConstraint(const VariableIdxVector& scope);
	ScopedSumConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~ScopedSumConstraint() {}
	
	virtual Filtering filteringType() { return Filtering::Custom; };
	
	bool isSatisfied(const ObjectIdxVector& values) const;
	
	//! Filters from the set of currently loaded projections
	Output filter();
	
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};


} // namespaces

