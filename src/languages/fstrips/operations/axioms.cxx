
#include <algorithm>

#include <languages/fstrips/operations/axioms.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <languages/fstrips/axioms.hxx>
#include <problem_info.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {

const ActionEffect* process_axioms(const ActionEffect& effect, const ProblemInfo& info) {
	const fs::Formula* condition = process_axioms(*effect.condition(), info);
	const Term* lhs = process_axioms(*effect.lhs(), info);
	const Term* rhs = process_axioms(*effect.rhs(), info);
	return new ActionEffect(lhs, rhs, condition);
}


const Formula* process_axioms(const Formula& element, const ProblemInfo& info) {
	FormulaAxiomVisitor visitor(info);
	element.Accept(visitor);
	return visitor._result;
}

std::vector<const Term*>
_process_subterms(const std::vector<const Term*>& subterms, const ProblemInfo& info) {
	std::vector<const Term*> result;
	for (auto unprocessed:subterms) {
		auto processed = process_axioms(*unprocessed, info);
		result.push_back(processed);
	}
	return result;
}

void FormulaAxiomVisitor::Visit(const Tautology& lhs) { _result = new Tautology; }
void FormulaAxiomVisitor::Visit(const Contradiction& lhs) { _result = new Contradiction; }


void FormulaAxiomVisitor::Visit(const AtomicFormula& lhs) {
	_result = lhs.clone(_process_subterms(lhs.getSubterms(), _info));
}

void FormulaAxiomVisitor::Visit(const AxiomaticFormula& lhs) {
	_result = lhs.clone(_process_subterms(lhs.getSubterms(), _info));
}

void FormulaAxiomVisitor::Visit(const AtomConjunction& lhs) { _result = lhs.clone();}


void FormulaAxiomVisitor::
Visit(const Conjunction& lhs) {
	std::vector<const Formula*> conjuncts;
	for (const Formula* c:lhs.getSubformulae()) {
		conjuncts.push_back(process_axioms(*c, _info));
	}
	_result =  new Conjunction(conjuncts);
}

void FormulaAxiomVisitor::
Visit(const Disjunction& lhs) {
	std::vector<const Formula*> disjuncts;
	for (const Formula* c:lhs.getSubformulae()) {
		disjuncts.push_back(process_axioms(*c, _info));
	}
	_result =  new Disjunction(disjuncts);
}


void FormulaAxiomVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	_result = new ExistentiallyQuantifiedFormula(Utils::clone(lhs.getVariables()), process_axioms(*lhs.getSubformula(), _info));
}

void FormulaAxiomVisitor::
Visit(const UniversallyQuantifiedFormula& lhs) {
	_result = new UniversallyQuantifiedFormula(Utils::clone(lhs.getVariables()), process_axioms(*lhs.getSubformula(), _info));
}




const Term* process_axioms(const Term& element, const ProblemInfo& info) {
	TermAxiomVisitor visitor(info);
	element.Accept(visitor);
	return visitor._result;
}

//! Nothing to be done, simply return a clone of the element
void TermAxiomVisitor::Visit(const StateVariable& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const Constant& lhs) { _result = lhs.clone(); }

void TermAxiomVisitor::Visit(const BoundVariable& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const NestedTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const ArithmeticTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const AdditionTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const SubtractionTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const MultiplicationTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const DivisionTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const PowerTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const SqrtTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const SineTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const CosineTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const TangentTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const ArcSineTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const ArcCosineTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const ArcTangentTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const ExpTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const MinTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const MaxTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const StaticHeadedNestedTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const AxiomaticTerm& lhs) { _result = lhs.clone(); }
void TermAxiomVisitor::Visit(const FluentHeadedNestedTerm& lhs) { _result = lhs.clone(); }

void TermAxiomVisitor::
Visit(const UserDefinedStaticTerm& lhs) {
	const auto& symbol_id = lhs.getSymbolId();
	const std::string& symbol = _info.getSymbolName(symbol_id);
	const Problem& problem = Problem::getInstance();
	if (const Axiom* axiom = problem.getAxiom(symbol)) {
		const auto& subterms = _process_subterms(lhs.getSubterms(), _info);
		// TODO - This should be creating an actual atom formula, not term
		// auto axiomaticAtom = new AxiomaticAtom(axiom, subterms);
		_result = new AxiomaticTermWrapper(axiom, symbol_id, subterms);
	} else {
		_result = lhs.clone();
	}
}

} } } // namespaces
