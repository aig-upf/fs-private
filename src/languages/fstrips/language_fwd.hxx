
#pragma once

//! Forward declarations of the FSTRIPS namespace

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
class ExternallyDefinedFormula;
class AxiomaticFormula;
class OpenFormula;
class AtomConjunction;
class RelationalFormula;
class EQAtomicFormula;
class NEQAtomicFormula;
class LTAtomicFormula;
class LEQAtomicFormula;
class GTAtomicFormula;
class GEQAtomicFormula;


// Terms
class Term;
class StateVariable;
class BoundVariable;
class Constant;
class IntConstant;
class NestedTerm;
class StaticHeadedNestedTerm;
class FluentHeadedNestedTerm;
class UserDefinedStaticTerm;
class AxiomaticTerm;
class ArithmeticTerm;
class AdditionTerm;
class SubtractionTerm;
class MultiplicationTerm;
	
} } } // namespaces

