
#pragma once

#include <languages/fstrips/base.hxx>
#include <fs_types.hxx> // TODO - REMOVE THIS DEPENDENCY

namespace fs0 { class ProblemInfo; }

namespace fs0 { namespace language { namespace fstrips {

enum class Connective { Conjunction, Disjunction, Negation };
const std::string to_string(Connective connective);

enum class Quantifier { Universal, Existential };
const std::string to_string(Quantifier connective);

//! A logical term in FSTRIPS
class LTerm : public LogicalElement {
public:
	LTerm() = default;
	virtual ~LTerm() = default;

	LTerm* clone() const override = 0;
};

class LogicalVariable : public LTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	LogicalVariable(unsigned id, TypeIdx type) : _id(id), _type(type) {}
	~LogicalVariable() = default;
	LogicalVariable(const LogicalVariable&) = default;

	LogicalVariable* clone() const override { return new LogicalVariable(*this); }
	
	//! Returns the unique quantified variable ID
	unsigned getId() const { return _id; }	
	
	//! Returns the ID of the variable type
	TypeIdx getType() const { return _type; }
	
	std::ostream& print(std::ostream& os, const ProblemInfo& info) const override;

protected:
	//! The ID of the variable, which will be unique throughout the whole binding unit.
	unsigned _id;
	
	//! The type of the variable
	TypeIdx _type;	
};

class LConstant : public LTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	LConstant(ObjectIdx value, TypeIdx type)  : _value(value), _type(type) {}
	~LConstant() = default;
	LConstant(const LConstant&) = default;

	LConstant* clone() const override { return new LConstant(*this); }
	
	//! Returns the unique quantified variable ID
	unsigned getValue() const { return _value; }	
	
	//! Returns the ID of the variable type
	TypeIdx getType() const { return _type; }
	
	std::ostream& print(std::ostream& os, const ProblemInfo& info) const override;

protected:
	//! The actual value of the constant
	ObjectIdx _value;
	
	//! The type of the constant
	TypeIdx _type;	
};


class FunctionalTerm : public LTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	FunctionalTerm(unsigned symbol_id, const std::vector<const LTerm*>& subterms) : _symbol_id(symbol_id), _children(subterms) {}
	~FunctionalTerm() = default;
	FunctionalTerm(const FunctionalTerm&);

	FunctionalTerm* clone() const override { return new FunctionalTerm(*this); }
	
	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<const LTerm*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const ProblemInfo& info) const override;

protected:
	//! The ID of the function symbol, e.g. in the state variable loc(A), the id of 'loc'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	const std::vector<const LTerm*> _children;
};


class LFormula : public LogicalElement {
public:
	LFormula() = default;
	virtual ~LFormula() = default;

	LFormula* clone() const override = 0;
};


//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class LAtomicFormula : public LFormula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	LAtomicFormula(unsigned symbol_id, const std::vector<const LTerm*>& subterms) : _symbol_id(symbol_id), _children(subterms) {}
	~LAtomicFormula() = default;
	LAtomicFormula(const LAtomicFormula&);

	LAtomicFormula* clone() const override { return new LAtomicFormula(*this); }

	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<const LTerm*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const ProblemInfo& info) const override;


protected:
	//! The ID of the predicate symbol, e.g. in the state variable clear(A), the id of 'clear'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	const std::vector<const LTerm*> _children;
};


//! A formula made of some logical connective applied to a number of subarguments
//! Will tipically be negation, conjunction, disjunction
class OpenLFormula : public LFormula {
public:
	
	OpenLFormula(Connective connective, const std::vector<const LFormula*>& subformulae) : _connective(connective), _children(subformulae) {}
	~OpenLFormula() { for (const auto ptr:_children) delete ptr; }
	OpenLFormula(const OpenLFormula&);

	Connective getConnective() const { return _connective; }
	
	const std::vector<const LFormula*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

protected:
	//! The actual logical connective of the open formula
	Connective _connective;
	
	//! The formula subformulae
	std::vector<const LFormula*> _children;
};


//! A formula quantified by at least one variable
class LQuantifiedFormula : public LFormula {
public:
	LQuantifiedFormula(Quantifier quantifier, const std::vector<const LogicalVariable*>& variables, const LFormula* subformula) : _quantifier(quantifier), _variables(variables), _subformula(subformula) {}

	virtual ~LQuantifiedFormula() {
		delete _subformula;
		for (auto ptr:_variables) delete ptr;
	}

	LQuantifiedFormula(const LQuantifiedFormula& other);

	const LFormula* getSubformula() const { return _subformula; }
	
	const std::vector<const LogicalVariable*>& getVariables() const { return _variables; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

protected:
	//! The actual type of quantifier
	Quantifier _quantifier;
	
	//! The binding IDs of the existentially quantified variables
	std::vector<const LogicalVariable*> _variables;

	//! ATM we only allow quantification of conjunctions
	const LFormula* _subformula;
};


} } } // namespaces

