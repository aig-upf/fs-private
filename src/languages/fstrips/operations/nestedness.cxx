
#include <algorithm>

#include <languages/fstrips/operations/nestedness.hxx>
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
	for (const auto* conjunct:lhs.getConjuncts()) _result = std::max(_result, nestedness(*conjunct));
}
	
void NestednessVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	_result = nestedness(*lhs.getSubformula());
}

} } } // namespaces
