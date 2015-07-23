
#pragma once

#include <constraints/scoped_constraint.hxx>
#include <constraints/scoped_effect.hxx>
#include <problem_info.hxx>
#include <boost/container/flat_map.hpp>
#include <functional>

// MRJ: This allows to switch between unordered_set and vector to represent constraints extensionally.
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
	static ExtensionT _compile(const UnaryParametrizedScopedConstraint& constraint);
	
public:
	typedef std::function<bool (ObjectIdx)> Tester;
	
	//! Construct a unary compiled constraint by compiling a standard unary constraint.
	CompiledUnaryConstraint(const UnaryParametrizedScopedConstraint& constraint);
	
	//!
	CompiledUnaryConstraint(const VariableIdxVector& scope, const Tester& tester);
	
	virtual ~CompiledUnaryConstraint() {};
	
	bool isSatisfied(ObjectIdx o) const;
	
	//! Filters from a new set of domains.
	Output filter(const DomainMap& domains) const;
	
	//! Compiled constraints cannot be compiled again!
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
	
	//! Returns a set with all tuples for the given scope that satisfy the the given state
	static std::unordered_set<ElementT> compile(const VariableIdxVector& scope, const Tester& tester);
	
	//! Helper to compile a standard unary constraint
	static std::unordered_set<ElementT> compile(const UnaryParametrizedScopedConstraint& constraint) {
		return compile(constraint.getScope(), [&constraint](ObjectIdx value){ return constraint.isSatisfied(value); });
	}
};


class CompiledBinaryConstraint : public BinaryParametrizedScopedConstraint
{
public:
	typedef std::set<std::tuple<ObjectIdx, ObjectIdx>> TupleExtension;
	typedef std::function<bool (ObjectIdx, ObjectIdx)> Tester;
	
protected:
	// For a binary constraint with scope <X, Y>, the extension is a map mapping each possible x \in D_X to an ordered 
	// vector containing all y \in D_Y s.t. <x, y> satisfies the constraint.
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		typedef std::unordered_map<ObjectIdx, ObjectIdxVector> ExtensionT;
	#else
		typedef std::unordered_set< ObjectIdx > ObjectIdxHash;
		typedef std::unordered_map< ObjectIdx, ObjectIdxHash > ExtensionT;
	#endif
	
	//! Precondition: the vector is sorted.
	const ExtensionT _extension1;
	const ExtensionT _extension2;
	
	
public:
	
	//!
	CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, const TupleExtension& extension);
	
	//! Construct a binary compiled constraint by compiling a standard binary constraint.
	CompiledBinaryConstraint(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo);
	
	CompiledBinaryConstraint(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester);
	
	virtual ~CompiledBinaryConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const;
	
	Output filter(unsigned variable);
	
	//! Compiled constraints cannot be compiled again!
	virtual ScopedConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
	
	static TupleExtension compile(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester);
	
	static ExtensionT index(const CompiledBinaryConstraint::TupleExtension& extension, unsigned variable);
	
	//! Returns a set with all tuples for the given scope that satisfy the the given state
// 	static std::map<ObjectIdx, std::set<ObjectIdx>> compile(const VariableIdxVector& scope, const Tester& tester);
	
	//! Helper to compile a standard unary constraint
	static TupleExtension compile(const fs0::BinaryParametrizedScopedConstraint& constraint) {
		return compile(constraint.getScope(), [&constraint](ObjectIdx x, ObjectIdx y){ return constraint.isSatisfied(x, y); });
	}
};



class CompiledUnaryEffect  {
protected:
	typedef ObjectIdx ElementT;
	typedef boost::container::flat_map<ElementT, ElementT> ExtensionT;
	
public:
	//! Returns a set with all values that satisfy the constraint
	static ExtensionT compile(const UnaryScopedEffect& effect, const ProblemInfo& problemInfo);
};


} // namespaces

