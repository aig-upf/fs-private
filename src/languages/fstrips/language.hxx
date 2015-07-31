
#pragma once
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <problem_info.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

class Term {
public:
	typedef const Term* cptr;
	
	Term() {}
	virtual ~Term() {}

	//! Clone idiom
	virtual Term* clone() const = 0;
	
	//! Returns the level of nestedness of the term.
	virtual unsigned nestedness() const = 0;
	
	//! Computes the term scope, i.e. a vector of state variables involved in the term.
	virtual VariableIdxVector computeScope() const;
	virtual void computeScope(std::set<VariableIdx>& scope) const = 0;
	
	//! Returns the value of the current term under the given (possibly partial) interpretation
	virtual ObjectIdx interpret(const PartialAssignment& assignment) const = 0;
	virtual ObjectIdx interpret(const State& state) const = 0;
	
	//! Returns the index of the state variable to which the current term resolves under the given state.
	virtual VariableIdx interpretVariable(const State& state) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Term& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class NestedTerm : public Term {
public:
	typedef const NestedTerm* cptr;
	
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
	
	using Term::computeScope; // necessary to make the other computeScope visible too
	void computeScope(std::set<VariableIdx>& scope) const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	//! A small helper
	template <typename T>
	static std::ostream& printFunction(std::ostream& os, const fs0::ProblemInfo& info, unsigned symbol_id, const std::vector<T*>& subterms) {
		os << info.getFunctionName(symbol_id) << "(";
		for (const T* term:subterms) os << *term << ", ";
		os << ")";
		return os;
	}
	
	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<Term::cptr>& getSubterms() const { return _subterms; }
	
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
};


//! A nested term headed by a static functional symbol
class StaticHeadedNestedTerm : public NestedTerm {
public:
	typedef const StaticHeadedNestedTerm* cptr;
	
	StaticHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms);
	
	StaticHeadedNestedTerm* clone() const { return new StaticHeadedNestedTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
	
	VariableIdx interpretVariable(const State& state) const { throw std::runtime_error("static-headed terms cannot resolve to an state variable"); }
	
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
	typedef const FluentHeadedNestedTerm* cptr;
	
	FluentHeadedNestedTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms)
		: NestedTerm(symbol_id, subterms) {}
	
	FluentHeadedNestedTerm* clone() const { return new FluentHeadedNestedTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
	
	VariableIdx interpretVariable(const State& state) const;
	
	// A nested term headed by a fluent symbol has as many levels of nestedness as the maximum of its subterms plus one (standing for itself)
	unsigned nestedness() const {
		return maxSubtermNestedness() + 1;
	}
	
	//! Computes the term scope ignoring the first level, i.e. the head of the term
	void computeSubtermScope(std::set<VariableIdx>& scope) const;
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
	ObjectIdx interpret(const State& state) const;
	
	VariableIdx interpretVariable(const State& state) const { return _variable_id; }
	
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
	ObjectIdx interpret(const State& state) const { { return _value; }}
	
	VariableIdx interpretVariable(const State& state) const { throw std::runtime_error("Constant terms cannot resolve to an state variable"); }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
protected:
	//! The actual value of the constant
	ObjectIdx _value;
};



class AtomicFormula {
public:
	typedef const AtomicFormula* cptr;
	
	enum class Symbol {EQ, NEQ, LT, LEQ, GT, GEQ};
	
	static AtomicFormula::cptr create(Symbol symbol, Term::cptr lhs, Term::cptr rhs);
	
	AtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : lhs(lhs_), rhs(rhs_), scope(computeScope()) {}
	
	virtual ~AtomicFormula() {
		delete lhs; delete rhs;
	}
		
	//! Returns the scope of the atomic formula, i.e. a vector/set with all state variables involved in it.
	void computeScope(std::set<VariableIdx>& scope) const;
	VariableIdxVector computeScope() const;
	
	bool interpret(const PartialAssignment& assignment) const { return _satisfied(lhs->interpret(assignment), rhs->interpret(assignment)); }
	bool interpret(const State& state) const { return _satisfied(lhs->interpret(state), rhs->interpret(state)); }
	
	virtual Symbol symbol() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const AtomicFormula& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;	
	
	Term::cptr lhs;
	Term::cptr rhs;
	
	const static std::map<AtomicFormula::Symbol, std::string> symbol_to_string;
	const static std::map<std::string, AtomicFormula::Symbol> string_to_symbol;
	
	const VariableIdxVector& getScope() const { return scope; }
	
	//! We cache the scope for performance reasons
	const VariableIdxVector scope;
	
protected:
	virtual bool _satisfied(ObjectIdx o1, ObjectIdx o2) const = 0;
};

class EQAtomicFormula : public AtomicFormula {
public:
	EQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 == o2; }
	
	virtual Symbol symbol() const { return Symbol::EQ; }
};

class NEQAtomicFormula : public AtomicFormula {
public:
	NEQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }
	
	Symbol symbol() const { return Symbol::NEQ; }
};

class LTAtomicFormula : public AtomicFormula {
public:
	LTAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }
	
	Symbol symbol() const { return Symbol::LT; }
};

class LEQAtomicFormula : public AtomicFormula {
public:
	LEQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	Symbol symbol() const { return Symbol::LEQ; }
};

class GTAtomicFormula : public AtomicFormula {
public:
	GTAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 > o2; }
	
	Symbol symbol() const { return Symbol::GT; }
};

class GEQAtomicFormula : public AtomicFormula {
public:
	GEQAtomicFormula(Term::cptr lhs_, Term::cptr rhs_) : AtomicFormula(lhs_, rhs_) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 >= o2; }
	
	Symbol symbol() const { return Symbol::GEQ; }
};


class ActionEffect {
public:
	typedef const ActionEffect* cptr;
	
	ActionEffect(Term::cptr lhs_, Term::cptr rhs_)
		: lhs(lhs_), rhs(rhs_),
		  scope(computeScope()),
		  affected(computeAffected())
	{
		if (!isWellFormed()) throw std::runtime_error("Ill-formed effect");
	}
	
	virtual ~ActionEffect() {
		delete lhs; delete rhs;
	}
	
	//! Checks that the 
	bool isWellFormed() const;
	
	//! Returns the scope of the effect
	void computeScope(std::set<VariableIdx>& scope) const;
	VariableIdxVector computeScope() const;
	
	//! Returns the index of the variable affected by the effect
	void computeAffected(std::set<VariableIdx>& affected) const;
	VariableIdxVector computeAffected() const;
	
	//!
	Atom apply(const State& state) const;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ActionEffect& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;

	
	Term::cptr lhs;
	Term::cptr rhs;
	
	//! We cache the scope / set of affected variables for performance reasons
	const VariableIdxVector scope;
	const VariableIdxVector affected;
};



} } } // namespaces
