
#pragma once

#include <fs/core/fs_types.hxx> //  TODO[LAMBDA] - REMOVE THIS DEPENDENCY ?

namespace fs0 { namespace fstrips {

class LanguageInfo;

//! A logical connective
enum class Connective { Conjunction, Disjunction, Negation };

//! A logical quantifier
enum class Quantifier { Universal, Existential };

//! Mappings from and to string representations of the above
const std::string to_string(Connective connective);
const std::string to_string(Quantifier quantifier);
const Connective to_connective(const std::string& connective);
const Quantifier to_quantifier(const std::string& quantifier);


///////////////////////////////////////////////////////////////////////////////
// A common base class for both terms and formulas
///////////////////////////////////////////////////////////////////////////////
class LogicalElement
{
public:
	virtual ~LogicalElement() = default;
	
	//! Clone idiom
	virtual LogicalElement* clone() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const LogicalElement& o);
	virtual std::ostream& print(std::ostream& os, const LanguageInfo& info) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
// Terms and Formulas
///////////////////////////////////////////////////////////////////////////////

//! A logical term in FSTRIPS
class Term : public LogicalElement {
public:
	Term() = default;
	virtual ~Term() = default;

	Term* clone() const override = 0;
};

class LogicalVariable : public Term {
public:
	LogicalVariable(unsigned id, const std::string& name, TypeIdx type) : _id(id), _name(name), _type(type) {}

	~LogicalVariable() = default;
	LogicalVariable(const LogicalVariable&) = default;

	LogicalVariable* clone() const override { return new LogicalVariable(*this); }
	
	//! Returns the unique quantified variable ID
	unsigned getId() const { return _id; }
	
	//! Returns the name of the variable
	const std::string& getName() const { return _name; }
	
	//! Returns the ID of the variable type
	TypeIdx getType() const { return _type; }
	
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

protected:
	//! The ID of the variable, which will be unique throughout the whole binding unit.
	unsigned _id;
	
	const std::string _name;
	
	//! The type of the variable
	TypeIdx _type;	
};

//! A constant is a term whose denotation is fixed throughout the whole planning process.
//! Note that this is not exactly a constant in classical FOL terms (where a constant is defined as a 0-ary function)
//! Thus we're making here a semantic distinction, but it is extremely useful to simplify computations.
class Constant : public Term {
public:
	Constant(object_id value, TypeIdx type)  : _value(value), _type(type) {}
	~Constant() = default;
	Constant(const Constant&) = default;

	Constant* clone() const override { return new Constant(*this); }
	
	//! 
	object_id getValue() const { return _value; }	
	
	//! Returns the ID of the variable type
	TypeIdx getType() const { return _type; }
	
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

protected:
	//! The actual value of the constant
	object_id _value;
	
	//! The type of the constant
	TypeIdx _type;	
};


class CompositeTerm : public Term {
public:
    CompositeTerm(unsigned symbol_id, const std::vector<const Term*>& subterms) : _symbol_id(symbol_id), _children(subterms) {}
	~CompositeTerm();
    CompositeTerm(const CompositeTerm&);

    CompositeTerm* clone() const override { return new CompositeTerm(*this); }
	
	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<const Term*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

protected:
	//! The ID of the function symbol, e.g. in the state variable loc(A), the id of 'loc'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	const std::vector<const Term*> _children;
};


class Formula : public LogicalElement {
public:
	Formula() = default;
	virtual ~Formula() = default;

	Formula* clone() const override = 0;
};

//! The True truth value
class Tautology : public Formula {
public:
	Tautology* clone() const override { return new Tautology; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override { return os << "True"; }
};

//! The False truth value
class Contradiction : public Formula {
public:
	Contradiction* clone() const override { return new Contradiction; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override { return os << "False"; }
};

//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class AtomicFormula : public Formula {
public:

	AtomicFormula(unsigned symbol_id, const std::vector<const Term*>& subterms) : _symbol_id(symbol_id), _children(subterms) {}
	~AtomicFormula();
	AtomicFormula(const AtomicFormula&);

	AtomicFormula* clone() const override { return new AtomicFormula(*this); }

	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<const Term*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;


protected:
	//! The ID of the predicate symbol, e.g. in the state variable clear(A), the id of 'clear'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	const std::vector<const Term*> _children;
};


//! A formula made of some logical connective applied to a number of subarguments
//! Will tipically be negation, conjunction, disjunction
class CompositeFormula : public Formula {
public:
	CompositeFormula(Connective connective, const std::vector<const Formula*>& subformulae) : _connective(connective), _children(subformulae) {}
	~CompositeFormula();
	CompositeFormula(const CompositeFormula&);
	
	CompositeFormula* clone() const override { return new CompositeFormula(*this); }

	Connective getConnective() const { return _connective; }
	
	const std::vector<const Formula*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

protected:
	//! The actual logical connective of the open formula
	Connective _connective;
	
	//! The formula subformulae
	std::vector<const Formula*> _children;
};


//! A formula quantified by at least one variable
class QuantifiedFormula : public Formula {
public:
	QuantifiedFormula(Quantifier quantifier, const std::vector<const LogicalVariable*>& variables, const Formula* subformula) : _quantifier(quantifier), _variables(variables), _subformula(subformula) {}

	~QuantifiedFormula();

	QuantifiedFormula(const QuantifiedFormula& other);
	QuantifiedFormula* clone() const override { return new QuantifiedFormula(*this); }

    Quantifier getQuantifier() const { return _quantifier; }

    const Formula* getSubformula() const { return _subformula; }
	
	const std::vector<const LogicalVariable*>& getVariables() const { return _variables; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const LanguageInfo& info) const override;

protected:
	//! The actual type of quantifier
	Quantifier _quantifier;
	
	//! The binding IDs of the existentially quantified variables
	std::vector<const LogicalVariable*> _variables;

	//! ATM we only allow quantification of conjunctions
	const Formula* _subformula;
};


} } // namespaces

