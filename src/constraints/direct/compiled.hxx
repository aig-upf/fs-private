
#pragma once

#include <unordered_map>
#include <set>
#include <functional>
#include <boost/container/flat_map.hpp>

#include <constraints/direct/constraint.hxx>
#include <constraints/direct/effect.hxx>


namespace fs0 { namespace language { namespace fstrips { class Term; } } }
namespace fs = fs0::language::fstrips;

namespace fs0 {

class CompiledUnaryConstraint : public UnaryDirectConstraint {
protected:
	typedef ObjectIdx ElementT;
	typedef std::vector<ElementT> ExtensionT;

	//! Precondition: the vector is sorted.
	const ExtensionT _extension;

	//! Protected constructor to be used from the other constructor
	CompiledUnaryConstraint(const VariableIdxVector& scope, const std::vector<ObjectIdx>& parameters, ExtensionT&& extension);

public:
	typedef std::function<bool (ObjectIdx)> Tester;

	//! Construct a unary compiled constraint by compiling a standard unary constraint.
	CompiledUnaryConstraint(const UnaryDirectConstraint& constraint);

	//!
	CompiledUnaryConstraint(const VariableIdxVector& scope, const Tester& tester);

	~CompiledUnaryConstraint() {};

	bool isSatisfied(ObjectIdx o) const override;

	//! Filters from a new set of domains.
	FilteringOutput filter(const DomainMap& domains) const override;

	//! Compiled constraints cannot be compiled again!
	DirectConstraint* compile(const ProblemInfo& info) const override { return nullptr; }

	//! Returns a set with all tuples for the given scope that satisfy the the given state
	static std::set<ElementT> compile(const VariableIdxVector& scope, const Tester& tester);

	//! Helper to compile a standard unary constraint
	static std::set<ElementT> compile(const UnaryDirectConstraint& constraint) {
		return compile(constraint.getScope(), [&constraint](ObjectIdx value){ return constraint.isSatisfied(value); });
	}

	std::ostream& print(std::ostream& os) const override;

protected:
	//! Returns an ordered ExtensionT data structure with all the elements that satisfy the constraint.
	static ExtensionT _compile(const UnaryDirectConstraint& constraint);

	static ExtensionT _compile(const VariableIdxVector& scope, const Tester& tester);
};


class CompiledBinaryConstraint : public BinaryDirectConstraint
{
public:
	typedef std::set<std::tuple<ObjectIdx, ObjectIdx>> TupleExtension;
	typedef std::function<bool (ObjectIdx, ObjectIdx)> Tester;

protected:
	// For a binary constraint with scope <X, Y>, the extension is a map mapping each possible x \in D_X to an ordered
	// vector containing all y \in D_Y s.t. <x, y> satisfies the constraint.
	typedef std::unordered_map<ObjectIdx, std::vector<ObjectIdx>> ExtensionT;

	//! Precondition: the vector is sorted.
	const ExtensionT _extension1;
	const ExtensionT _extension2;


public:

	//!
	CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<ObjectIdx>& parameters, const TupleExtension& extension);

	//! Construct a binary compiled constraint by compiling a standard binary constraint.
	CompiledBinaryConstraint(const BinaryDirectConstraint& constraint, const ProblemInfo& problemInfo);

	CompiledBinaryConstraint(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester);

	~CompiledBinaryConstraint() {};

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const override;

	FilteringOutput filter(unsigned variable) const override;

	//! Compiled constraints cannot be compiled again!
	DirectConstraint* compile(const ProblemInfo& info) const override { return nullptr; }

	static TupleExtension compile(const VariableIdxVector& scope, const CompiledBinaryConstraint::Tester& tester);

	static ExtensionT index(const CompiledBinaryConstraint::TupleExtension& extension, unsigned variable);

	//! Returns a set with all tuples for the given scope that satisfy the the given state
// 	static std::map<ObjectIdx, std::set<ObjectIdx>> compile(const VariableIdxVector& scope, const Tester& tester);

	//! Helper to compile a standard unary constraint
	static TupleExtension compile(const fs0::BinaryDirectConstraint& constraint) {
		return compile(constraint.getScope(), [&constraint](ObjectIdx x, ObjectIdx y){ return constraint.isSatisfied(x, y); });
	}

	std::ostream& print(std::ostream& os) const override;
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
	static unsigned compileConstraints(std::vector<DirectConstraint*>& constraints);
};


} // namespaces
