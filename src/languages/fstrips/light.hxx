
#pragma once

#include <languages/fstrips/base.hxx>
#include <fs_types.hxx> // TODO - REMOVE THIS DEPENDENCY

namespace fs0 { class ProblemInfo; } // TODO - REMOVE THIS DEPENDENCY

namespace fs0 { namespace lang { namespace fstrips {

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
// Terms and Formulas
///////////////////////////////////////////////////////////////////////////////

//! A logical term in FSTRIPS
class Term : public language::fstrips::LogicalElement {
public:
	Term() = default;
	virtual ~Term() = default;

	Term* clone() const override = 0;
};

class LogicalVariable : public Term {
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

//! A constant is a term whose denotation is fixed throughout the whole planning process.
//! Note that this is not exactly a constant in classical FOL terms (where a constant is defined as a 0-ary function)
//! Thus we're making here a semantic distinction, but it is extremely useful to simplify computations.
class Constant : public Term {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	Constant(ObjectIdx value, TypeIdx type)  : _value(value), _type(type) {}
	~Constant() = default;
	Constant(const Constant&) = default;

	Constant* clone() const override { return new Constant(*this); }
	
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


class FunctionalTerm : public Term {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	FunctionalTerm(unsigned symbol_id, const std::vector<const Term*>& subterms) : _symbol_id(symbol_id), _children(subterms) {}
	~FunctionalTerm() = default;
	FunctionalTerm(const FunctionalTerm&);

	FunctionalTerm* clone() const override { return new FunctionalTerm(*this); }
	
	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<const Term*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const ProblemInfo& info) const override;

protected:
	//! The ID of the function symbol, e.g. in the state variable loc(A), the id of 'loc'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	const std::vector<const Term*> _children;
};


class Formula : public language::fstrips::LogicalElement {
public:
	Formula() = default;
	virtual ~Formula() = default;

	Formula* clone() const override = 0;
};

//! The True truth value
class Tautology : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	Tautology* clone() const override { return new Tautology; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override { return os << "True"; }
};

//! The False truth value
class Contradiction : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	Contradiction* clone() const override { return new Contradiction; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override { return os << "False"; }
};

//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class AtomicFormula : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	AtomicFormula(unsigned symbol_id, const std::vector<const Term*>& subterms) : _symbol_id(symbol_id), _children(subterms) {}
	~AtomicFormula() = default;
	AtomicFormula(const AtomicFormula&);

	AtomicFormula* clone() const override { return new AtomicFormula(*this); }

	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<const Term*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const ProblemInfo& info) const override;


protected:
	//! The ID of the predicate symbol, e.g. in the state variable clear(A), the id of 'clear'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	const std::vector<const Term*> _children;
};


//! A formula made of some logical connective applied to a number of subarguments
//! Will tipically be negation, conjunction, disjunction
class OpenFormula : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	OpenFormula(Connective connective, const std::vector<const Formula*>& subformulae) : _connective(connective), _children(subformulae) {}
	~OpenFormula() { for (const auto ptr:_children) delete ptr; }
	OpenFormula(const OpenFormula&);
	
	OpenFormula* clone() const override { return new OpenFormula(*this); }

	Connective getConnective() const { return _connective; }
	
	const std::vector<const Formula*>& getChildren() const { return _children; }

	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

protected:
	//! The actual logical connective of the open formula
	Connective _connective;
	
	//! The formula subformulae
	std::vector<const Formula*> _children;
};


//! A formula quantified by at least one variable
class QuantifiedFormula : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	QuantifiedFormula(Quantifier quantifier, const std::vector<const LogicalVariable*>& variables, const Formula* subformula) : _quantifier(quantifier), _variables(variables), _subformula(subformula) {}

	virtual ~QuantifiedFormula() {
		delete _subformula;
		for (auto ptr:_variables) delete ptr;
	}

	QuantifiedFormula(const QuantifiedFormula& other);
	QuantifiedFormula* clone() const override { return new QuantifiedFormula(*this); }


	const Formula* getSubformula() const { return _subformula; }
	
	const std::vector<const LogicalVariable*>& getVariables() const { return _variables; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

protected:
	//! The actual type of quantifier
	Quantifier _quantifier;
	
	//! The binding IDs of the existentially quantified variables
	std::vector<const LogicalVariable*> _variables;

	//! ATM we only allow quantification of conjunctions
	const Formula* _subformula;
};


///////////////////////////////////////////////////////////////////////////////
// Effects, Actions
///////////////////////////////////////////////////////////////////////////////


//! The effect of a planning (grounded) action, which is of the form
//!     LHS := RHS
//! where both LHS (left-hand side) and RHS (right-hand side) are terms in our language,
//! with the particularity that LHS must be either a state variable or a fluent-headed nested term.
class ActionEffect {
public:
	ActionEffect(const Formula* condition) : _condition(condition) {}
	
	virtual ~ActionEffect() { delete _condition; };
	
	ActionEffect(const ActionEffect&);
	ActionEffect(ActionEffect&&) = default;
	ActionEffect& operator=(const ActionEffect&) = delete;
	ActionEffect& operator=(ActionEffect&&) = default;
	
	virtual ActionEffect* clone() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;

	//! Accessor
	const Formula* condition() const { return _condition; }
	
	virtual bool is_predicative() const = 0;
	virtual bool is_add() const = 0;
	virtual bool is_del() const = 0;
	
protected:
	
	//! The effect condition
	const Formula* _condition;
};


class FunctionalEffect : public ActionEffect {
public:
	FunctionalEffect(const FunctionalTerm* lhs, const Term* rhs, const Formula* condition)
		: ActionEffect(condition), _lhs(lhs), _rhs(rhs) {}
	virtual ~FunctionalEffect() { delete _lhs; delete _rhs; };
	
	FunctionalEffect(const FunctionalEffect&);
	FunctionalEffect(FunctionalEffect&&) = default;
	FunctionalEffect& operator=(const FunctionalEffect&) = delete;
	FunctionalEffect& operator=(FunctionalEffect&&) = default;
	
	FunctionalEffect* clone() const override { return new FunctionalEffect(*this); }
	
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

	//! Accessors for the left-hand side and right-hand side of the effect
	const Term* lhs() const { return _lhs; }
	const Term* rhs() const { return _rhs; }
	
	bool is_predicative() const override { return false; };
	bool is_add() const override { return false; };
	bool is_del() const override { return false; };
	
protected:
	//! The LHS _must_ be a functional term, i.e. we cannot modify the
	//! denotation of (i.e. assign to) a constant or a logical variable
	const FunctionalTerm* _lhs;
	
	//! The effect RHS
	const Term* _rhs;
};

class AtomicEffect : public ActionEffect {
public:
	enum class Type {ADD, DEL};
	static Type to_type(const std::string& type);
	
	AtomicEffect(const AtomicFormula* atom, Type type, const Formula* condition)
		: ActionEffect(condition), _atom(atom), _type(type) {}
	virtual ~AtomicEffect() { delete _atom; };
	
	AtomicEffect(const AtomicEffect&);
	AtomicEffect(AtomicEffect&&) = default;
	AtomicEffect& operator=(const AtomicEffect&) = delete;
	AtomicEffect& operator=(AtomicEffect&&) = default;
	
	AtomicEffect* clone() const override { return new AtomicEffect(*this); }
	
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

	const AtomicFormula* getAtom() const { return _atom; }
	const Type getType() const { return _type; }
	
	bool is_predicative() const override { return true; };
	bool is_add() const override { return _type == Type::ADD; }
	bool is_del() const override { return _type == Type::DEL; }
	
protected:
	//! An AtomicEffect involves an atomic formula
	const AtomicFormula* _atom;
	
	//! The type (add/del) of atomic effect
	const Type _type;
};



class ActionSchema {
protected:
	//! The ID of the original action schema (not to be confused with the ID of resulting fully-grounded actions)
	unsigned _id;
	
	const std::string _name;
	const Signature _signature;
	const std::vector<std::string> _parameter_names;
	const Formula* _precondition;
	const std::vector<const ActionEffect*> _effects;

public:
	ActionSchema(unsigned id, const std::string& name, const Signature& signature, const std::vector<std::string>& parameter_names,
                 const Formula* precondition, const std::vector<const ActionEffect*>& effects);
	
	~ActionSchema();
	ActionSchema(const ActionSchema&);
	
	unsigned getId() const { return _id; }
	const std::string& getName() const { return _name; }
	const Signature& getSignature() const { return _signature; }
	const std::vector<std::string>& getParameterNames() const { return _parameter_names; }
	const Formula* getPrecondition() const { return _precondition; }
	const std::vector<const ActionEffect*>& getEffects() const { return _effects; }
	
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionSchema& entity) { return entity.print(os); }
	std::ostream& print(std::ostream& os) const;
};






} } } // namespaces

