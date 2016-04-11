
#pragma once

#include <languages/fstrips/language.hxx>
#include <constraints/direct/constraint.hxx>
#include <constraints/direct/effect.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

//! A direct translator compiles term-based formulae and effects into direct constraints / effects
class DirectTranslator {
public:
	//! Generates the DirectConstraint that corresponds to a given (atomic) formula
	static DirectConstraint* generate(const fs::AtomicFormula& formula);
	static DirectConstraint* generate(const fs::RelationalFormula& formula);
	
	//! Generates the set of DirectConstraints that corresponds to a given set of (atomic) formulae
	static std::vector<DirectConstraint*> generate(const std::vector<fs::AtomicFormula::cptr> formulae);
	
	//! Generates the DirectEffect that corresponds to a given language effect
	static const DirectEffect* generate(const fs::ActionEffect& effect);
	
	//! Generates a set of DirectEffects that correspond to the given language effects
	static std::vector<const DirectEffect*> generate(const std::vector<fs::ActionEffect::cptr>& effects);
	
protected:
	
	//! Helper to instantiate unary constraints
	static DirectConstraint* instantiateUnaryConstraint(fs::RelationalFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters, bool invert);
	
	//! Helper to instantiate binary constraints
	static DirectConstraint* instantiateBinaryConstraint(fs::RelationalFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	//! Perform some basic checks to ensure that the term is compatible with direct components
	static void checkSupported(const fs::Term::cptr lhs, const fs::Term::cptr rhs);
	
	//! Transforms a given formula into an extensional constraint, if the scope is small enough.
	//! Otherwise returns a null pointer.
	static DirectConstraint* extensionalize(const fs::AtomicFormula& formula);
};

} // namespaces
