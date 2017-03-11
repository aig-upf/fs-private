
#include <languages/fstrips/operations/conjunction.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {


Formula* conjunction(const Formula& lhs, const Formula& rhs) { 
	BootstrappingConjunctionVisitor bootstrapper;
	lhs.Accept(bootstrapper);
	ConjunctionVisitor& visitor = *bootstrapper._visitor;
	rhs.Accept(visitor);
// 	std::cout << "\"" << lhs << "\"" <<" && " << "\"" << rhs << "\"" << " = " << *visitor._result << std::endl;
	return visitor._result;
}



///////////////////////////////////////////////////////////////////////////////////
// LHS: TAUTOLOGY
///////////////////////////////////////////////////////////////////////////////////
void TautologyLhsConjunctionVisitor::
Visit(const Formula& rhs) { _result = rhs.clone(); }

void TautologyLhsConjunctionVisitor::
Visit(const Tautology& rhs) { _result = rhs.clone(); }

void TautologyLhsConjunctionVisitor::
Visit(const Contradiction& rhs) { _result = rhs.clone(); }

void TautologyLhsConjunctionVisitor::
Visit(const AtomicFormula& rhs) { _result = rhs.clone(); }

void TautologyLhsConjunctionVisitor::
Visit(const Conjunction& rhs) { _result = rhs.clone(); }

void TautologyLhsConjunctionVisitor::
Visit(const ExistentiallyQuantifiedFormula& rhs) { _result = rhs.clone(); }


///////////////////////////////////////////////////////////////////////////////////
// LHS: CONTRADICTION
///////////////////////////////////////////////////////////////////////////////////
void ContradictionLhsConjunction::
Visit(const Formula& rhs) { _result = new Contradiction; }

void ContradictionLhsConjunction::
Visit(const Tautology& rhs) { _result = new Contradiction; }

void ContradictionLhsConjunction::
Visit(const Contradiction& rhs) { _result = new Contradiction; }

void ContradictionLhsConjunction::
Visit(const AtomicFormula& rhs) { _result = new Contradiction; }

void ContradictionLhsConjunction::
Visit(const Conjunction& rhs) { _result = new Contradiction; }

void ContradictionLhsConjunction::
Visit(const ExistentiallyQuantifiedFormula& rhs) { _result = new Contradiction; }


///////////////////////////////////////////////////////////////////////////////////
// LHS: ATOMIC FORMULA
///////////////////////////////////////////////////////////////////////////////////
void AtomicFormulaLhsConjunction::
Visit(const Formula& rhs) { throw UnimplementedFeatureException(""); }

void AtomicFormulaLhsConjunction::
Visit(const Tautology& rhs) { _result = _lhs.clone(); }

void AtomicFormulaLhsConjunction::
Visit(const Contradiction& rhs) { throw UnimplementedFeatureException(""); }

void AtomicFormulaLhsConjunction::
Visit(const AtomicFormula& rhs) { throw UnimplementedFeatureException(""); }

void AtomicFormulaLhsConjunction::
Visit(const Conjunction& rhs) { throw UnimplementedFeatureException(""); }

void AtomicFormulaLhsConjunction::
Visit(const ExistentiallyQuantifiedFormula& rhs) { throw UnimplementedFeatureException(""); }


///////////////////////////////////////////////////////////////////////////////////
// LHS: CONJUNCTION
///////////////////////////////////////////////////////////////////////////////////
void ConjunctionLhsConjunction::
Visit(const Formula& rhs) { throw UnimplementedFeatureException(""); }

void ConjunctionLhsConjunction::
Visit(const Tautology& rhs) { _result = _lhs.clone(); }

void ConjunctionLhsConjunction::
Visit(const Contradiction& rhs) { _result = new Contradiction; }

void ConjunctionLhsConjunction::
Visit(const AtomicFormula& rhs) { throw UnimplementedFeatureException(""); }

void ConjunctionLhsConjunction::
Visit(const Conjunction& rhs) { 
	auto all_subterms = Utils::merge(Utils::clone(_lhs.getSubformulae()), Utils::clone(rhs.getSubformulae()));
	_result = new Conjunction(all_subterms);
}

void ConjunctionLhsConjunction::
Visit(const ExistentiallyQuantifiedFormula& rhs) {
	// We simply return the existentially quantified formula that results from conjuncting the LHS subconjunction with the RHS conjunction, with the same quantified variables.
	_result = new ExistentiallyQuantifiedFormula(rhs.getVariables(), conjunction(_lhs, *rhs.getSubformula()));
}


///////////////////////////////////////////////////////////////////////////////////
// LHS: EXISTENTIALLY-QUANTIFIED FORMULA
///////////////////////////////////////////////////////////////////////////////////
void ExistentiallyQuantifiedFormulaLhsConjunction::
Visit(const Formula& rhs) { throw UnimplementedFeatureException(""); }

void ExistentiallyQuantifiedFormulaLhsConjunction::
Visit(const Tautology& rhs) { _result = _lhs.clone(); }

void ExistentiallyQuantifiedFormulaLhsConjunction::
Visit(const Contradiction& rhs) { _result = new Contradiction; }

void ExistentiallyQuantifiedFormulaLhsConjunction::
Visit(const AtomicFormula& rhs) { throw UnimplementedFeatureException(""); }

void ExistentiallyQuantifiedFormulaLhsConjunction::
Visit(const Conjunction& rhs) {
	// We simply return the existentially quantified formula that results from conjuncting the LHS subconjunction with the RHS conjunction, with the same quantified variables.
	_result = new ExistentiallyQuantifiedFormula(_lhs.getVariables(), conjunction(rhs, *_lhs.getSubformula()));
}

void ExistentiallyQuantifiedFormulaLhsConjunction::
Visit(const ExistentiallyQuantifiedFormula& rhs) { throw UnimplementedFeatureException(""); }


} } } // namespaces
