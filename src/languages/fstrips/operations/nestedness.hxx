
#pragma once

#include <utils/visitor.hxx>
#include <languages/fstrips/formulae.hxx>

//!
//!
//!

namespace fs0 { namespace language { namespace fstrips {
	
/*
An arbitrary ordering among types:
Formula
Tautology
Contradiction
AtomicFormula
Conjunction
ExistentiallyQuantifiedFormula
*/


//! The main operation: a conjunction between two arbitrary formulas.
unsigned nestedness(const Formula& element);


class NestednessVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Formula, void, true>
    , public Loki::Visitor<Tautology, void, true>
	, public Loki::Visitor<Contradiction, void, true>
	, public Loki::Visitor<AtomicFormula, void, true>
	, public Loki::Visitor<Conjunction, void, true>
	, public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
{
public:
	NestednessVisitor() = default;
	~NestednessVisitor() = default;
	
 	void Visit(const Formula& lhs) override { throw UnimplementedFeatureException(""); }
	void Visit(const Tautology& lhs) override { _result = 0; }
	void Visit(const Contradiction& lhs) override { _result = 0; }
	void Visit(const AtomicFormula& lhs) override;
	void Visit(const Conjunction& lhs) override;
	void Visit(const ExistentiallyQuantifiedFormula& lhs) override;

	unsigned _result;
};

} } } // namespaces

