
#pragma once

#include <constraints/scoped_constraint.hxx>
#include <problem_info.hxx>


namespace fs0 {



class CompiledUnaryConstraint : public UnaryParametrizedScopedConstraint {
protected:
	typedef ObjectIdx ElementT;
	typedef std::vector<ElementT> ExtensionT;
	
	//! Precondition: the vector is sorted.
	const ExtensionT _extension;

	//! Protected constructor to be used from the other constructor
	CompiledUnaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension);
	
	//! Returns an ordered ExtensionT data structure with all the elements that satisfy the constraint.
	ExtensionT compile(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
public:
	//! Construct a unary compiled constraint by compiling a standard unary constraint.
	CompiledUnaryConstraint(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
	virtual ~CompiledUnaryConstraint() {};
	
	bool isSatisfied(ObjectIdx o) const;
	
	//! Filters from a new set of domains.
	Output filter(const DomainMap& domains) const;
};

class CompiledBinaryConstraint : public BinaryParametrizedScopedConstraint
{
protected:
	typedef std::pair<ObjectIdx, ObjectIdx> ElementT;
	typedef std::vector<ElementT> ExtensionT;
	
	//! Precondition: the vector is sorted.
	const ExtensionT _extension;
	
	//! Protected constructor to be used from the other constructor
	CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension);
	
	//! Returns an ordered ExtensionT data structure with all the elements that satisfy the constraint.
	ExtensionT compile(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
public:
	//! Construct a binary compiled constraint by compiling a standard binary constraint.
	CompiledBinaryConstraint(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
	virtual ~CompiledBinaryConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const;
	
	Output filter(unsigned variable);
};


} // namespaces

