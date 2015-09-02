
#pragma once

#include <constraints/direct/constraint.hxx>
#include <constraints/direct/effect.hxx>
#include <boost/container/flat_map.hpp>
#include <unordered_map>
#include <functional>

// MRJ: This allows to switch between unordered_set and vector to represent constraints extensionally.
#ifndef EXTENSIONAL_REPRESENTATION_USES_VECTORS 
	#include <unordered_set>
#endif



namespace fs0 { namespace language { namespace fstrips {
	class Term;
} } }

namespace fs = fs0::language::fstrips;

namespace fs0 {

class CompiledUnaryConstraint : public UnaryDirectConstraint {
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
	static ExtensionT _compile(const UnaryDirectConstraint& constraint);
	
public:
	typedef std::function<bool (ObjectIdx)> Tester;
	
	//! Construct a unary compiled constraint by compiling a standard unary constraint.
	CompiledUnaryConstraint(const UnaryDirectConstraint& constraint);
	
	//!
	CompiledUnaryConstraint(const VariableIdxVector& scope, const Tester& tester);
	
	~CompiledUnaryConstraint() {};
	
	bool isSatisfied(ObjectIdx o) const;
	
	//! Filters from a new set of domains.
	FilteringOutput filter(const DomainMap& domains) const;
	
	//! Compiled constraints cannot be compiled again!
	DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
	
	//! Returns a set with all tuples for the given scope that satisfy the the given state
	static std::unordered_set<ElementT> compile(const VariableIdxVector& scope, const Tester& tester);
	
	//! Helper to compile a standard unary constraint
	static std::unordered_set<ElementT> compile(const UnaryDirectConstraint& constraint) {
		return compile(constraint.getScope(), [&constraint](ObjectIdx value){ return constraint.isSatisfied(value); });
	}
};


class CompiledBinaryConstraint : public BinaryDirectConstraint
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
	CompiledBinaryConstraint(const BinaryDirectConstraint& constraint, const ProblemInfo& problemInfo);
	
	CompiledBinaryConstraint(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester);
	
	~CompiledBinaryConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const;
	
	FilteringOutput filter(unsigned variable);
	
	//! Compiled constraints cannot be compiled again!
	DirectConstraint::cptr compile(const ProblemInfo& problemInfo) const { return nullptr; }
	
	static TupleExtension compile(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester);
	
	static ExtensionT index(const CompiledBinaryConstraint::TupleExtension& extension, unsigned variable);
	
	//! Returns a set with all tuples for the given scope that satisfy the the given state
// 	static std::map<ObjectIdx, std::set<ObjectIdx>> compile(const VariableIdxVector& scope, const Tester& tester);
	
	//! Helper to compile a standard unary constraint
	static TupleExtension compile(const fs0::BinaryDirectConstraint& constraint) {
		return compile(constraint.getScope(), [&constraint](ObjectIdx x, ObjectIdx y){ return constraint.isSatisfied(x, y); });
	}
};



class CompiledUnaryEffect : public UnaryDirectEffect {
protected:
	typedef ObjectIdx ElementT;
	typedef boost::container::flat_map<ElementT, ElementT> ExtensionT;
	
	// The extension is a map x -> y, implicitly encoding the current effect y := f(x)
	ExtensionT _extension;
	
	//! Protected constructor, to be invoked from the factory method
	CompiledUnaryEffect(VariableIdx relevant, VariableIdx affected, ExtensionT&& extension);

public:
	//! Construct a Compiled unary effect from a given logical term
	CompiledUnaryEffect(VariableIdx relevant, VariableIdx affected, const fs::Term& term);
	
	Atom apply(ObjectIdx value) const;
	
	//! Returns a set with all values that satisfy the constraint
	static ExtensionT compile(const UnaryDirectEffect& effect, const ProblemInfo& info);
	static ExtensionT compile(const fs::Term& term, const ProblemInfo& info);
};

class CompiledBinaryEffect : public BinaryDirectEffect {
protected:
	typedef ObjectIdx ElementT;
	typedef boost::container::flat_map<std::pair<ElementT, ElementT>, ElementT> ExtensionT;
	
	// The extension is a map (x1, x2) -> y, implicitly encoding the current effect y := f(x1, x2)
	ExtensionT _extension;
	
	//! Protected constructor, to be invoked from the factory method
	CompiledBinaryEffect(const VariableIdxVector& scope, VariableIdx affected, ExtensionT&& extension);

public:
	//! Construct a Compiled unary effect from a given logical term
	CompiledBinaryEffect(const VariableIdxVector& scope, VariableIdx affected, const fs::Term& term);
	
	Atom apply(ObjectIdx v1, ObjectIdx v2) const;
	
	//! Returns a set with all values that satisfy the constraint
	static ExtensionT compile(const fs::Term& term, const ProblemInfo& info);
};

class ConstraintCompiler {
public:
	// In-place compilation of a set of constraints. Returns the number of compiled constraints.
	static unsigned compileConstraints(std::vector<DirectConstraint::cptr>& constraints);
};


} // namespaces

