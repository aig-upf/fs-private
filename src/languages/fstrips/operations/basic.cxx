
#include <algorithm>

#include <languages/fstrips/operations/basic.hxx>
#include <languages/fstrips/terms.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {


unsigned nestedness(const Formula& element) {
	NestednessVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}

void NestednessVisitor::
Visit(const AtomicFormula& lhs) {
	_result = 0;
	for (const Term* subterm:lhs.getSubterms()) _result = std::max(_result, subterm->nestedness());
}

void NestednessVisitor::	
Visit(const Conjunction& lhs) {
	_result = 0;
	for (const auto* conjunct:lhs.getSubformulae()) _result = std::max(_result, nestedness(*conjunct));
}
	
void NestednessVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	_result = nestedness(*lhs.getSubformula());
}







std::vector<const Formula*> all_formulae(const Formula& element) {
	AllFormulaVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}


std::vector<const AtomicFormula*> all_atoms(const Formula& element) {
	return Utils::filter_by_type<const AtomicFormula*>(all_formulae(element));
}

std::vector<const Term*> all_terms(const Formula& element) {
	std::vector<const Term*> res;
	
	for (const AtomicFormula* atom:fs::all_atoms(element)) {
		for (const Term* term:atom->getSubterms()) {
			auto tmp = term->all_terms();
			res.insert(res.end(), tmp.cbegin(), tmp.cend());
		}
	}
	
	return res;
}



} } } // namespaces
