
#pragma once

#include <constraints/scoped_constraint.hxx>
#include <problem_info.hxx>
// MRJ: This allows to switch between unordered_set and vector to represent extensionally
// constraints.
#ifndef EXTENSIONAL_REPRESENTATION_USES_VECTORS 
	#include <unordered_set>
#endif

namespace fs0 {



class CompiledUnaryConstraint : public UnaryParametrizedScopedConstraint {
protected:
	typedef ObjectIdx ElementT;
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		typedef std::vector<ElementT> ExtensionT;
	#else
		typedef std::unordered_set<ElementT> ExtensionT;
	#endif
	
	//! Precondition: the vector is sorted.
	const ExtensionT _extension;

	//! Protected constructor to be used from the other constructor
	CompiledUnaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension);
	
	//! Returns an ordered ExtensionT data structure with all the elements that satisfy the constraint.
	ExtensionT _compile(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
public:
	//! Construct a unary compiled constraint by compiling a standard unary constraint.
	CompiledUnaryConstraint(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
	virtual ~CompiledUnaryConstraint() {};
	
	bool isSatisfied(ObjectIdx o) const;
	
	//! Filters from a new set of domains.
	Output filter(const DomainMap& domains) const;
	
	//! Compiled constraints cannot be compiled
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};

class CompiledBinaryConstraint : public BinaryParametrizedScopedConstraint
{
protected:
	// For a binary constraint with scope <X, Y>, the extension is a map mapping each possible x \in D_X to an ordered 
	// vector containing all y \in D_Y s.t. <x, y> satisfies the constraint.
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		typedef std::unordered_map<ObjectIdx, ObjectIdxVector> ExtensionT;
	#else
		typedef std::unordered_set< ObjectIdx >	ObjectIdxHash;
		typedef std::unordered_map< ObjectIdx, ObjectIdxHash > ExtensionT;
	#endif
	
	//! Precondition: the vector is sorted.
	const ExtensionT _extension1;
	const ExtensionT _extension2;
	
	//! Protected constructor to be used from the other constructor
	CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension1, ExtensionT&& extension2);
	
	//! Returns an ordered ExtensionT data structure with all the elements that satisfy the constraint.
	ExtensionT _compile(const BinaryParametrizedScopedConstraint& constraint, unsigned variable, const ProblemInfo& problemInfo);
	
public:
	//! Construct a binary compiled constraint by compiling a standard binary constraint.
	CompiledBinaryConstraint(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
	virtual ~CompiledBinaryConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const;
	
	Output filter(unsigned variable);
	
	//! Compiled constraints cannot be compiled
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
};


} // namespaces

