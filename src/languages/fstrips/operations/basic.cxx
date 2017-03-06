
#include <algorithm>

#include <languages/fstrips/operations/basic.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {


unsigned nestedness(const LogicalElement& element) {
	NestednessVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}

void NestednessVisitor::
Visit(const AtomicFormula& lhs) {
	_result = 0;
	for (const Term* subterm:lhs.getSubterms()) _result = std::max(_result, nestedness(*subterm));
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

//! A private helper
unsigned _maxSubtermNestedness(const std::vector<const Term*>& subterms) {
	unsigned max = 0;
	for (const Term* subterm:subterms) max = std::max(max, nestedness(*subterm));
	return max;
}
	
void NestednessVisitor::
Visit(const StaticHeadedNestedTerm& lhs) {
	// A nested term headed by a static symbol has as many levels of nestedness as the maximum of its subterms
	_result = _maxSubtermNestedness(lhs.getSubterms());
}

void NestednessVisitor::
Visit(const FluentHeadedNestedTerm& lhs) {
	// A nested term headed by a fluent symbol has as many levels of nestedness as the maximum of its subterms plus one (standing for itself)
	_result = _maxSubtermNestedness(lhs.getSubterms()) + 1;
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




void AllFormulaVisitor::
Visit(const Tautology& lhs) { _result.push_back(&lhs); }

void AllFormulaVisitor::
Visit(const Contradiction& lhs) { _result.push_back(&lhs); }

void AllFormulaVisitor::
Visit(const AtomicFormula& lhs) { _result.push_back(&lhs); }

void AllFormulaVisitor::
Visit(const Conjunction& lhs) {
	_result.push_back(&lhs);
	for (auto elem:lhs.getSubformulae()) {
		auto tmp = all_formulae(*elem);
		_result.insert(_result.end(), tmp.cbegin(), tmp.cend());
	}
}

void AllFormulaVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	_result.push_back(&lhs);
	auto tmp = all_formulae(*lhs.getSubformula());
	_result.insert(_result.end(), tmp.cbegin(), tmp.cend());
}





} } } // namespaces
