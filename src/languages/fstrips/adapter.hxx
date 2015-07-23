
#pragma once

#include <languages/fstrips/language.hxx>
#include <constraints/scoped_constraint.hxx>


namespace fs0 { namespace language { namespace fstrips {

class Adapter {
public:
	//!
	static std::vector<ScopedConstraint::cptr> generate(const AtomicFormula& formula);
	
protected:
	//!
	static std::vector<ScopedConstraint::cptr> generate0(const AtomicFormula& formula);
	
	//!
// 	static std::tuple<ObjectIdxVector, VariableIdxVector> classify(const std::vector<Term::cptr>& terms);
	
	//!
	static ScopedConstraint::cptr instantiateUnaryConstraint(AtomicFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	//!
	static ScopedConstraint::cptr instantiateBinaryConstraint(AtomicFormula::Symbol symbol, const VariableIdxVector& scope, const std::vector<int>& parameters);
	
};

} } } // namespaces
