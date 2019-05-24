
#pragma once

//! Forward declarations of the most relevant elements of the FOL syntax namespace
namespace fs0::fstrips {

//! The base for terms and formulas
class LogicalElement;

//! Terms
class Term;
class LogicalVariable;
class Constant;
class CompositeTerm;

//! Formulas
class Formula;
class Tautology;
class Contradiction;
class AtomicFormula;
class QuantifiedFormula;
class CompositeFormula;
enum class Connective;
enum class Quantifier;

} // namespaces

