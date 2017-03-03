
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


//! The level of nestedness of a given formula
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

////////////////////////////////////////////////////////////

//! Returns a vector with all the subformulae involved in the current formula
std::vector<const Formula*> all_formulae(const Formula& element);

//! Returns a vector with all the terms involved in the current formula
std::vector<const Term*> all_terms(const Formula& element);

//! A small helper - returns a vector with all the atomic formulae involved in the current formula
std::vector<const AtomicFormula*> all_atoms(const Formula& element);


class AllFormulaVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Formula, void, true>
    , public Loki::Visitor<Tautology, void, true>
	, public Loki::Visitor<Contradiction, void, true>
	, public Loki::Visitor<AtomicFormula, void, true>
	, public Loki::Visitor<Conjunction, void, true>
	, public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
{
public:
	AllFormulaVisitor() = default;
	~AllFormulaVisitor() = default;
	
 	void Visit(const Formula& lhs) override { throw UnimplementedFeatureException(""); }
	void Visit(const Tautology& lhs) override { _result.push_back(&lhs); }
	void Visit(const Contradiction& lhs) override { _result.push_back(&lhs); }
	void Visit(const AtomicFormula& lhs) override { _result.push_back(&lhs); }
	void Visit(const Conjunction& lhs) override {
		_result.push_back(&lhs);
		for (auto elem:lhs.getSubformulae()) {
			auto tmp = all_formulae(*elem);
			_result.insert(_result.end(), tmp.cbegin(), tmp.cend());
		}
	}

	void Visit(const ExistentiallyQuantifiedFormula& lhs) override {
		_result.push_back(&lhs);
		auto tmp = all_formulae(*lhs.getSubformula());
		_result.insert(_result.end(), tmp.cbegin(), tmp.cend());
	}

	std::vector<const Formula*> _result;
};



} } } // namespaces

