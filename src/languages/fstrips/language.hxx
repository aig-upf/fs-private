
#pragma once
#include <fs0_types.hxx>


namespace fs0 { class ProblemInfo; }

namespace fs0 { namespace language { namespace fstrips {

class Term {
public:
	typedef const Term* cptr;
	
	Term() {}
	virtual ~Term() = 0;

	//! Clone idiom
	virtual Term* clone() const = 0;
	
	//! Returns the level of nestedness of the term.
	virtual unsigned nestedness() const = 0;
	
	//! Returns the term scope, i.e. a vector of state variables involved in the term.
	void computeScope(VariableIdxVector& scope) const;
	virtual void computeScope(std::set<VariableIdx>& scope) const = 0;
	
	//! Returns the value of the current term under the given (possibly partial) interpretation
	virtual ObjectIdx interpret(const PartialAssignment& assignment) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Term& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class NestedTerm : public Term {
public:
	NestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
		: _symbol_id(symbol_id), _subterms(subterms)
	{}
	
	virtual ~NestedTerm() {
		for (Term::cptr term:_subterms) delete term;
	}
	
	NestedTerm(const NestedTerm& term)
		: _symbol_id(term._symbol_id) {
		for (const Term* subterm:term._subterms) {
			_subterms.push_back(subterm->clone());
		}
	}
	
	void computeScope(std::set<VariableIdx>& scope) const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
protected:
	//! The ID of the function or predicate symbol, e.g. in the state variable loc(A), the id of 'loc'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	std::vector<Term::cptr> _subterms;
	
	unsigned maxSubtermNestedness() const {
		unsigned max = 0;
		for (Term::cptr subterm:_subterms) max = std::max(max, subterm->nestedness());
		return max;
	}
	
	ObjectIdxVector interpret_subterms(const PartialAssignment& assignment) const;
};


//! A nested term headed by a static functional symbol
class StaticHeadedNestedTerm : public NestedTerm {
public:
	StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms);
	
	StaticHeadedNestedTerm* clone() const { return new StaticHeadedNestedTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	
	// A nested term headed by a static symbol has as many levels of nestedness as the maximum of its subterms
	virtual unsigned nestedness() const {
		return maxSubtermNestedness();
	}
protected:
	// The (static) logical function implementation
	const Function& _function;
};


//! A nested term headed by a fluent functional symbol
class FluentHeadedNestedTerm : public NestedTerm {
public:
	FluentHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
		: NestedTerm(symbol_id, subterms) {}
	
	FluentHeadedNestedTerm* clone() const { return new FluentHeadedNestedTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	
	// A nested term headed by a fluent symbol has as many levels of nestedness as the maximum of its subterms plus one (standing for itself)
	virtual unsigned nestedness() const {
		return maxSubtermNestedness() + 1;
	}
};

//! A state variable is a term 'f(t)', where f is a fluent symbol and t is a tuple of fixed constant symbols.
//! 'loc(a)', with a being an object, for instance, is a state variable
class StateVariable : public Term {
public:
	typedef const StateVariable* cptr;
	
	StateVariable(VariableIdx variable_id) : _variable_id(variable_id) {}
	
	StateVariable* clone() const { return new StateVariable(*this); }
	
	virtual unsigned nestedness() const { return 0; }
	
	//! A constant term has no scope.
	virtual void computeScope(std::set<VariableIdx>& scope) const;
	
	//! Returns the index of the state variable
	VariableIdx getValue() const { return _variable_id; }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const { return assignment.at(_variable_id); }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
protected:
	//! The ID of the state variable
	VariableIdx _variable_id;
};


//! A simple constant
class Constant : public Term {
public:
	typedef const Constant* cptr;
	
	Constant(ObjectIdx value)  : _value(value) {}
	
	Constant* clone() const { return new Constant(*this); }
	
	virtual unsigned nestedness() const { return 0; }
	
	//! A constant term has no scope.
	virtual void computeScope(std::set<VariableIdx>& scope) const { return; }
	
	//! Returns the actual value of the constant
	ObjectIdx getValue() const { return _value; }
	
	// The value of a constant is independent of the assignment
	ObjectIdx interpret(const PartialAssignment& assignment) const { return _value; }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
protected:
	//! The actual value of the constant
	ObjectIdx _value;
};






//! A term that has not yet been processed, meaning that it might possibly contain references to yet-unresolved values of action parameters,
//! non-consolidated state variables, etc.
class TermSchema {
public:
	typedef const TermSchema* cptr;
	
	TermSchema() {}
	virtual ~TermSchema() = 0;

	virtual TermSchema* clone() const = 0;
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	virtual Term* process(const ObjectIdxVector& binding) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const TermSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
};


class NestedTermSchema : public TermSchema {
public:
	NestedTermSchema(unsigned symbol_id, const std::vector<TermSchema::cptr>& subterms)
		: _symbol_id(symbol_id), _subterms(subterms)
	{}
	
	virtual ~NestedTermSchema() {
		for (TermSchema::cptr term:_subterms) delete term;
	}
	
	NestedTermSchema(const NestedTermSchema& term)
		: _symbol_id(term._symbol_id) {
		for (TermSchema::cptr subterm:term._subterms) {
			_subterms.push_back(subterm->clone());
		}
	}
	
	NestedTermSchema* clone() const { return new NestedTermSchema(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term* process(const ObjectIdxVector& binding) const;	
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The ID of the function or predicate symbol, e.g. in the state variable loc(A), the id of 'loc'
	unsigned _symbol_id;
	
	//! The tuple of fixed, constant symbols of the state variable, e.g. {A, B} in the state variable 'on(A,B)'
	std::vector<TermSchema::cptr> _subterms;
};


//! A state variable whose value depends on a certain binding of values to parameters, e.g. parameters in an action schema.
class ActionSchemaParameter : public TermSchema {
public:
	ActionSchemaParameter(unsigned position) : _position(position) {}
	
	ActionSchemaParameter* clone() const { return new ActionSchemaParameter(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term* process(const ObjectIdxVector& binding) const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The position of the parameter within the ordered set of action parameters
	unsigned _position;
};

//! A simple constant
class ConstantSchema : public TermSchema {
public:
	ConstantSchema(ObjectIdx value)  : _value(value) {}
	
	ConstantSchema* clone() const { return new ConstantSchema(*this); }
	
	//! Processes a possibly nested unprocessed term, consolidating the existing state variables
	//! and binding action parameters to concrete language constants.
	Term* process(const ObjectIdxVector& binding) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const ProblemInfo& info) const;
	
protected:
	//! The actual value of the constant
	ObjectIdx _value;
};


class AtomicFormula {
public:
	typedef const AtomicFormula* cptr;
	
	enum class RelationSymbol {EQ, NEQ, LT, LEQ, GT, GEQ};
	
	AtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : lhs(lhs_), rhs(rhs_) {}
	
	virtual ~AtomicFormula() {
		delete lhs; delete rhs;
	}
		
	//! Returns the scope of the atomic formula, i.e. a vector with all state variables involved in it.
	void computeScope(VariableIdxVector& scope) const;
	
	bool interpret(const PartialAssignment& assignment) const { return _satisfied(lhs->interpret(assignment), rhs->interpret(assignment)); }
	
	virtual RelationSymbol symbol() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const AtomicFormula& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;	
	
	Term::cptr lhs;
	Term::cptr rhs;
protected:
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const;
};

class EQAtomicFormula : public AtomicFormula {
public:
	EQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 = o2; }
	
	virtual RelationSymbol symbol() const { return RelationSymbol::EQ; }
	
	std::string symbol_str() const { return "="; }
};

class NEQAtomicFormula : public AtomicFormula {
public:
	NEQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }
	
	RelationSymbol symbol() const { return RelationSymbol::NEQ; }
};

class LTAtomicFormula : public AtomicFormula {
public:
	LTAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }
	
	RelationSymbol symbol() const { return RelationSymbol::LT; }
};

class LEQAtomicFormula : public AtomicFormula {
public:
	LEQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	RelationSymbol symbol() const { return RelationSymbol::LEQ; }
};

class GTAtomicFormula : public AtomicFormula {
public:
	GTAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 > o2; }
	
	RelationSymbol symbol() const { return RelationSymbol::GT; }
};

class GEQAtomicFormula : public AtomicFormula {
public:
	GEQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 >= o2; }
	
	RelationSymbol symbol() const { return RelationSymbol::GEQ; }
};


class AtomicFormulaSchema {
public:
	typedef const AtomicFormulaSchema* cptr;
	
	AtomicFormulaSchema(TermSchema::cptr lhs_, TermSchema::cptr rhs_, AtomicFormula::RelationSymbol symbol_) : lhs(lhs_), rhs(rhs_), symbol(symbol_) {}
	
	virtual ~AtomicFormulaSchema() {
		delete lhs; delete rhs;
	}
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const AtomicFormulaSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;	
	
	TermSchema::cptr lhs;
	TermSchema::cptr rhs;
	AtomicFormula::RelationSymbol symbol;
};


class ActionEffect {
public:
	typedef const ActionEffect* cptr;
	
	ActionEffect(Term::cptr lhs_, Term::cptr rhs_) : lhs(lhs_), rhs(rhs_) {}
	
	virtual ~ActionEffect() {
		delete lhs; delete rhs;
	}
		
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	Term::cptr lhs;
	Term::cptr rhs;
protected:
};

class ActionEffectSchema {
public:
	typedef const ActionEffectSchema* cptr;
	
	ActionEffectSchema(TermSchema::cptr lhs_, TermSchema::cptr rhs_) : lhs(lhs_), rhs(rhs_) {}
	
	virtual ~ActionEffectSchema() {
		delete lhs; delete rhs;
	}
		
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffectSchema& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	TermSchema::cptr lhs;
	TermSchema::cptr rhs;
protected:
};

} } } // namespaces
