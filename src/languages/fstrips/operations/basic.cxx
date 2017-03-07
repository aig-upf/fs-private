
#include <algorithm>

#include <languages/fstrips/operations/basic.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <utils/utils.hxx>
#include <problem_info.hxx>

namespace fs0 { namespace language { namespace fstrips {

std::vector<const Formula*> all_formulae(const Formula& element) {
	return Utils::filter_by_type<const Formula*>(all_nodes(element));
}

std::vector<const AtomicFormula*> all_atoms(const Formula& element) {
	return Utils::filter_by_type<const AtomicFormula*>(all_nodes(element));
}

std::vector<const Term*> all_terms(const LogicalElement& element) {
	return Utils::filter_by_type<const Term*>(all_nodes(element));
}


std::vector<const LogicalElement*> all_nodes(const LogicalElement& element) {
	AllNodesVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}

void AllNodesVisitor::
Visit(const Tautology& lhs) {
	_result.push_back(&lhs);
}

void AllNodesVisitor::
Visit(const Contradiction& lhs) {
	_result.push_back(&lhs);
}

void AllNodesVisitor::
Visit(const AtomicFormula& lhs) {
	_result.push_back(&lhs);
	for (const Term* subterm:lhs.getSubterms()) {
		subterm->Accept(*this);
	}
}

void AllNodesVisitor::
Visit(const Conjunction& lhs) {
	_result.push_back(&lhs);
	for (auto element:lhs.getSubformulae()) {
		element->Accept(*this);
	}
}

void AllNodesVisitor::
Visit(const ExistentiallyQuantifiedFormula& lhs) {
	_result.push_back(&lhs);
	for (auto element:lhs.getVariables()) {
		element->Accept(*this);
	}	
	lhs.getSubformula()->Accept(*this);
}
	
void AllNodesVisitor::Visit(const StateVariable& lhs) { _result.push_back(&lhs); }
void AllNodesVisitor::Visit(const BoundVariable& lhs) { _result.push_back(&lhs); }
void AllNodesVisitor::Visit(const Constant& lhs) { _result.push_back(&lhs); }
void AllNodesVisitor::Visit(const StaticHeadedNestedTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }
void AllNodesVisitor::Visit(const FluentHeadedNestedTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }
void AllNodesVisitor::Visit(const UserDefinedStaticTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }
void AllNodesVisitor::Visit(const AdditionTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }
void AllNodesVisitor::Visit(const SubtractionTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }
void AllNodesVisitor::Visit(const MultiplicationTerm& lhs) { Visit(static_cast<const NestedTerm&>(lhs)); }

void AllNodesVisitor::Visit(const NestedTerm& lhs) {
	_result.push_back(&lhs);
	for (const Term* subterm:lhs.getSubterms()) {
		subterm->Accept(*this);
	}
}


///////////////////////////////////////////////////////////////////////////////

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

void NestednessVisitor::Visit(const UserDefinedStaticTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void NestednessVisitor::Visit(const AdditionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void NestednessVisitor::Visit(const SubtractionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void NestednessVisitor::Visit(const MultiplicationTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }


unsigned flat(const Term& element) {
	FlatVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}


void FlatVisitor::Visit(const UserDefinedStaticTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void FlatVisitor::Visit(const AdditionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void FlatVisitor::Visit(const SubtractionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void FlatVisitor::Visit(const MultiplicationTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }	


///////////////////////////////


unsigned type(const Term& element) {
	TypeVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}


void TypeVisitor::
Visit(const StaticHeadedNestedTerm& lhs) { 
	_result = ProblemInfo::getInstance().getSymbolData(lhs.getSymbolId()).getCodomainType();
}

void TypeVisitor::
Visit(const FluentHeadedNestedTerm& lhs) { 
	_result = ProblemInfo::getInstance().getSymbolData(lhs.getSymbolId()).getCodomainType();
}

void TypeVisitor::
Visit(const UserDefinedStaticTerm& lhs) { 
	_result = lhs.getFunction().getCodomainType();
}

void TypeVisitor::
Visit(const BoundVariable& lhs) { 
	_result = lhs.getType();
}

void TypeVisitor::
Visit(const StateVariable& lhs) { 
	_result = ProblemInfo::getInstance().getVariableType(lhs.getValue());
}

void TypeVisitor::Visit(const AdditionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void TypeVisitor::Visit(const SubtractionTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
void TypeVisitor::Visit(const MultiplicationTerm& lhs) { Visit(static_cast<const StaticHeadedNestedTerm&>(lhs)); }
	

///////////////////////////////

std::pair<int, int> bounds(const Term& element) {
	BoundVisitor visitor;
	element.Accept(visitor);
	return visitor._result;
}

std::pair<int, int> type_based_bounds(const Term& element) {
	return ProblemInfo::getInstance().getTypeBounds(type(element));
}



void BoundVisitor::Visit(const StateVariable& lhs) { _result = type_based_bounds(lhs); }
void BoundVisitor::Visit(const BoundVariable& lhs) { _result = type_based_bounds(lhs); }
void BoundVisitor::Visit(const StaticHeadedNestedTerm& lhs) { _result = type_based_bounds(lhs); }
void BoundVisitor::Visit(const FluentHeadedNestedTerm& lhs) { _result = type_based_bounds(lhs); }
void BoundVisitor::Visit(const UserDefinedStaticTerm& lhs) { _result = type_based_bounds(lhs); }

void BoundVisitor::
Visit(const Constant& lhs) { 
	_result = std::make_pair(lhs.getValue(), lhs.getValue());
}

void BoundVisitor::
Visit(const AdditionTerm& lhs) { 
	// see https://en.wikipedia.org/wiki/Interval_arithmetic
	auto b0 = fs::bounds(*lhs.getSubterms()[0]);
	auto b1 = fs::bounds(*lhs.getSubterms()[1]);
	auto min = b0.first + b1.first; 
	auto max = b0.second + b1.second; 	
	_result = std::make_pair(min, max);
}

void BoundVisitor::
Visit(const SubtractionTerm& lhs) { 
	// see https://en.wikipedia.org/wiki/Interval_arithmetic
	auto b0 = fs::bounds(*lhs.getSubterms()[0]);
	auto b1 = fs::bounds(*lhs.getSubterms()[1]);
	auto min = b0.first - b1.second; 
	auto max = b0.second - b1.first; 	
	_result = std::make_pair(min, max);
}

void BoundVisitor::
Visit(const MultiplicationTerm& lhs) { 
	// see https://en.wikipedia.org/wiki/Interval_arithmetic
	auto b0 = fs::bounds(*lhs.getSubterms()[0]);
	auto b1 = fs::bounds(*lhs.getSubterms()[1]);
	std::vector<int> all{b0.first * b1.first, b0.first * b1.second, b0.second * b1.first, b0.second * b1.second};
	auto minmax = std::minmax_element(all.begin(), all.end());
	_result = std::make_pair(*minmax.first, *minmax.second);
}


} } } // namespaces
