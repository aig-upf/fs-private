
#pragma once

#include <utils/visitor.hxx>

//!
//!
//!

namespace fs0 { namespace language { namespace fstrips {

class LogicalElement;

// Formulas
class Formula;
class Tautology;
class Contradiction;
class AtomicFormula;
class Conjunction;
class Disjunction;
class Negation;
class ExistentiallyQuantifiedFormula;

// Terms
class Term;
class StateVariable;
class BoundVariable;
class Constant;
class NestedTerm;
class StaticHeadedNestedTerm;
class FluentHeadedNestedTerm;


//! The level of nestedness of a given formula
unsigned nestedness(const LogicalElement& element);


class NestednessVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<Tautology, void, true>
    , public Loki::Visitor<Contradiction, void, true>
    , public Loki::Visitor<AtomicFormula, void, true>
    , public Loki::Visitor<Conjunction, void, true>
    , public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
    
    , public Loki::Visitor<StateVariable, void, true>
    , public Loki::Visitor<BoundVariable, void, true>
    , public Loki::Visitor<Constant, void, true>
    , public Loki::Visitor<StaticHeadedNestedTerm, void, true>
    , public Loki::Visitor<FluentHeadedNestedTerm, void, true>
{
public:
	NestednessVisitor() = default;
	~NestednessVisitor() = default;
	
	void Visit(const Tautology& lhs) { _result = 0; }
	void Visit(const Contradiction& lhs) { _result = 0; }
	void Visit(const AtomicFormula& lhs);
	void Visit(const Conjunction& lhs);
	void Visit(const ExistentiallyQuantifiedFormula& lhs);
	
	
	void Visit(const StateVariable& lhs) { _result = 0; }
	void Visit(const BoundVariable& lhs) { _result = 1; }
	void Visit(const Constant& lhs) { _result = 0; }
	void Visit(const StaticHeadedNestedTerm& lhs);
	void Visit(const FluentHeadedNestedTerm& lhs);
	

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
    , public Loki::Visitor<Tautology, void, true>
    , public Loki::Visitor<Contradiction, void, true>
    , public Loki::Visitor<AtomicFormula, void, true>
    , public Loki::Visitor<Conjunction, void, true>
    , public Loki::Visitor<ExistentiallyQuantifiedFormula, void, true>
{
public:
	AllFormulaVisitor() = default;
	~AllFormulaVisitor() = default;
	
	void Visit(const Tautology& lhs);
	void Visit(const Contradiction& lhs);
	void Visit(const AtomicFormula& lhs);
	void Visit(const Conjunction& lhs);
	void Visit(const ExistentiallyQuantifiedFormula& lhs);

	std::vector<const Formula*> _result;
};


////////////////////////////////////////////////////////////
//! Returns true if the element is flat, i.e. is a state variable or a constant
////////////////////////////////////////////////////////////
unsigned flat(const Term& element);

class FlatVisitor
    : public Loki::BaseVisitor
    , public Loki::Visitor<StateVariable, void, true>
    , public Loki::Visitor<BoundVariable, void, true>
    , public Loki::Visitor<Constant, void, true>
    , public Loki::Visitor<StaticHeadedNestedTerm, void, true>
    , public Loki::Visitor<FluentHeadedNestedTerm, void, true>
{
public:
	void Visit(const StateVariable& lhs) { _result = true; }
	void Visit(const BoundVariable& lhs) { _result = true; }
	void Visit(const Constant& lhs) { _result = true; }
	void Visit(const StaticHeadedNestedTerm& lhs) { _result = false; }
	void Visit(const FluentHeadedNestedTerm& lhs) { _result = false; }

	bool _result;
};


} } } // namespaces

