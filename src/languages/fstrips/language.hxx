
#pragma once
#include <fs0_types.hxx>
#include <atoms.hxx>
#include <problem_info.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

//! A common base class for both terms and formulas
//! This is necessary in order to be able to use both types of entities in a common registry
class LogicalElement {
public:
	typedef const LogicalElement* cptr;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const LogicalElement& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;
};

//! A logical term in FSTRIPS
class Term : public LogicalElement {
public:
	typedef const Term* cptr;
	
	Term() {}
	virtual ~Term() {}

	//! Clone idiom
	virtual Term* clone() const = 0;
	
	//! Returns the level of nestedness of the term.
	virtual unsigned nestedness() const = 0;
	
	//! Returns true if the element is flat, i.e. is a state variable or a constant
	virtual bool flat() const = 0;
	
	//! Computes the term scope, i.e. a vector of state variables involved in the term.
	virtual VariableIdxVector computeScope() const;
	virtual void computeScope(std::set<VariableIdx>& scope) const = 0;
	
	//! Returns the value of the current term under the given (possibly partial) interpretation
	virtual ObjectIdx interpret(const PartialAssignment& assignment) const = 0;
	virtual ObjectIdx interpret(const State& state) const = 0;
	
	//! Returns the index of the state variable to which the current term resolves under the given state.
	virtual VariableIdx interpretVariable(const PartialAssignment& assignment) const = 0;
	virtual VariableIdx interpretVariable(const State& state) const = 0;
	
	virtual std::pair<int, int> getBounds() const = 0;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	virtual bool operator==(const Term& other) const = 0;
	inline bool operator!=(const Term& rhs) const { return !this->operator==(rhs); }
	virtual std::size_t hash_code() const = 0;
};

//! A nested logical term in FSTRIPS, i.e. a term of the form f(t_1, ..., t_n)
//! The class is abstract and intended to have two possible subclasses, depending on whether
//! the functional symbol 'f' is fluent or not.
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
	
	bool flat() const { return false; }
	
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
	
	template <typename T>
	static ObjectIdxVector interpret_subterms(const std::vector<Term::cptr>& subterms, const T& assignment);
	
	unsigned getSymbolId() const { return _symbol_id; }
	
	const std::vector<Term::cptr>& getSubterms() const { return _subterms; }
	
	bool operator==(const Term& other) const;
	virtual std::size_t hash_code() const;
	
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
	
	virtual ObjectIdx interpret(const PartialAssignment& assignment) const = 0;
	virtual ObjectIdx interpret(const State& state) const = 0;
	
	VariableIdx interpretVariable(const PartialAssignment& assignment) const { throw std::runtime_error("static-headed terms cannot resolve to an state variable"); }
	VariableIdx interpretVariable(const State& state) const { throw std::runtime_error("static-headed terms cannot resolve to an state variable"); }
	
	// A nested term headed by a static symbol has as many levels of nestedness as the maximum of its subterms
	unsigned nestedness() const { return maxSubtermNestedness(); }
};

//! A statically-headed term defined extensionally or otherwise by the concrete planning instance
class UserDefinedStaticTerm : public StaticHeadedNestedTerm {
public:
	typedef const UserDefinedStaticTerm* cptr;
	
	UserDefinedStaticTerm(unsigned symbol_id, const std::vector<Term::cptr>& subterms);
	
	UserDefinedStaticTerm* clone() const { return new UserDefinedStaticTerm(*this); }
	
	virtual std::pair<int, int> getBounds() const;
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
	
protected:
	// The (static) logical function implementation
	const FunctionData& _function;
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
	
	VariableIdx interpretVariable(const PartialAssignment& assignment) const;
	VariableIdx interpretVariable(const State& state) const;
	
	virtual std::pair<int, int> getBounds() const;
	
	//! The scope of a nested term headed by a fluent includes all possible variables
	//! resulting from the different possible values of its subterms
	void computeScope(std::set<VariableIdx>& scope) const;
	
	//! Return all state variables in which possible instantiations of the subterms might result
	void computeTopLevelScope(std::set<VariableIdx>& scope) const;
	
	// A nested term headed by a fluent symbol has as many levels of nestedness as the maximum of its subterms plus one (standing for itself)
	unsigned nestedness() const { return maxSubtermNestedness() + 1; }
	
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
	
	bool flat() const { return true; }
	
	//! A constant term has no scope.
	virtual void computeScope(std::set<VariableIdx>& scope) const;
	
	//! Returns the index of the state variable
	VariableIdx getValue() const { return _variable_id; }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const { return assignment.at(_variable_id); }
	ObjectIdx interpret(const State& state) const;
	
	VariableIdx interpretVariable(const PartialAssignment& assignment) const { return _variable_id; }
	VariableIdx interpretVariable(const State& state) const { return _variable_id; }
	
	virtual std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	bool operator==(const Term& other) const;
	virtual std::size_t hash_code() const;
	
protected:
	//! The ID of the state variable
	VariableIdx _variable_id;
};


//! A simple constant term.
class Constant : public Term {
public:
	typedef const Constant* cptr;
	
	Constant(ObjectIdx value)  : _value(value) {}
	
	Constant* clone() const { return new Constant(*this); }
	
	virtual unsigned nestedness() const { return 0; }
	
	bool flat() const { return true; }
	
	//! A constant term has no scope.
	virtual void computeScope(std::set<VariableIdx>& scope) const { return; }
	
	//! Returns the actual value of the constant
	ObjectIdx getValue() const { return _value; }
	
	// The value of a constant is independent of the assignment
	ObjectIdx interpret(const PartialAssignment& assignment) const { return _value; }
	ObjectIdx interpret(const State& state) const { { return _value; }}
	
	VariableIdx interpretVariable(const PartialAssignment& assignment) const { throw std::runtime_error("Constant terms cannot resolve to an state variable"); }
	VariableIdx interpretVariable(const State& state) const { throw std::runtime_error("Constant terms cannot resolve to an state variable"); }
	
	virtual std::pair<int, int> getBounds() const { return std::make_pair(_value, _value); }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	bool operator==(const Term& other) const;
	virtual std::size_t hash_code() const;
	
protected:
	//! The actual value of the constant
	ObjectIdx _value;
};

/*
//! An integer constant
class IntConstant : public Constant {
public:
	typedef const IntConstant* cptr;
	
	IntConstant(ObjectIdx value)  : Constant(value) {}
	
	IntConstant* clone() const { return new IntConstant(*this); }
	
	virtual std::pair<int, int> getBounds() const { return std::make_pair(_value, _value); }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};
*/


//! An atomic formula, implicitly understood to be static (fluent formulae are considered terms with Boolean codomain)
class AtomicFormula : public LogicalElement {
public:
	typedef const AtomicFormula* cptr;
	
	AtomicFormula(const std::vector<Term::cptr>& subterms) : _subterms(subterms), _scope(computeScope()) {}
	
	virtual ~AtomicFormula() {
		for (const auto ptr:_subterms) delete ptr;
	}
	
	const std::vector<Term::cptr>& getSubterms() const { return _subterms; }
	
	bool interpret(const PartialAssignment& assignment) const;
	bool interpret(const State& state) const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;
	
	const VariableIdxVector& getScope() const { return _scope; }
	
	//! By default, formulae are not tautology nor contradiction
	virtual bool is_tautology() const { return false; }
	virtual bool is_contradiction() const { return false; }
	
protected:
	//! Returns the scope of the atomic formula, i.e. a vector/set with all state variables involved in it.
	void computeScope(std::set<VariableIdx>& scope) const;
	VariableIdxVector computeScope() const;
	
	//! The formula subterms
	std::vector<Term::cptr> _subterms;
	
	//! We cache the scope for performance reasons
	const VariableIdxVector _scope;
	
	//! A helper to recursively evaluate the formula - must be subclassed
	virtual bool _satisfied(const ObjectIdxVector& values) const = 0;
};

//! The True truth value
class TrueFormula : public AtomicFormula {
public:
	typedef const TrueFormula* cptr;
	
	TrueFormula() : AtomicFormula({}) {}
	
	bool interpret(const PartialAssignment& assignment) const { return true; }
	bool interpret(const State& state) const { return true; }
	bool _satisfied(const ObjectIdxVector& values) const { return true; }
	
	bool is_tautology() const { return true; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const { os << "True"; return os; }
};

//! The False truth value
class FalseFormula : public AtomicFormula {
public:
	typedef const FalseFormula* cptr;
	
	FalseFormula() : AtomicFormula({}) {}
	
	bool interpret(const PartialAssignment& assignment) const { return false; }
	bool interpret(const State& state) const { return false; }
	bool _satisfied(const ObjectIdxVector& values) const { return false; }
	
	bool is_contradiction() const { return true; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const { os << "False"; return os; }
};

//! A formula of the form t_1 <op> t_2, where t_i are terms and <op> is a basic relational
//! operator such as =, !=, >, etc.
class RelationalFormula : public AtomicFormula {
public:
	typedef const RelationalFormula* cptr;
	
	enum class Symbol {EQ, NEQ, LT, LEQ, GT, GEQ};
	
	RelationalFormula(const std::vector<Term::cptr>& subterms) : AtomicFormula(subterms) {
		assert(subterms.size() == 2);
	}
	
	virtual Symbol symbol() const = 0;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;	
	
	
	const static std::map<RelationalFormula::Symbol, std::string> symbol_to_string;
	const static std::map<std::string, RelationalFormula::Symbol> string_to_symbol;
	
	const Term::cptr lhs() const { return _subterms[0]; }
	const Term::cptr rhs() const { return _subterms[1]; }
	
protected:
	bool _satisfied(const ObjectIdxVector& values) const { return _satisfied(values[0], values[1]); }
	virtual bool _satisfied(ObjectIdx o1, ObjectIdx o2) const = 0;
};

class EQAtomicFormula : public RelationalFormula {
public:
	EQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 == o2; }
	
	virtual Symbol symbol() const { return Symbol::EQ; }
};

class NEQAtomicFormula : public RelationalFormula {
public:
	NEQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }
	
	Symbol symbol() const { return Symbol::NEQ; }
};

class LTAtomicFormula : public RelationalFormula {
public:
	LTAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }
	
	Symbol symbol() const { return Symbol::LT; }
};

class LEQAtomicFormula : public RelationalFormula {
public:
	LEQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	Symbol symbol() const { return Symbol::LEQ; }
};

class GTAtomicFormula : public RelationalFormula {
public:
	GTAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 > o2; }
	
	Symbol symbol() const { return Symbol::GT; }
};

class GEQAtomicFormula : public RelationalFormula {
public:
	GEQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 >= o2; }
	
	Symbol symbol() const { return Symbol::GEQ; }
};

//! The effect of a planning (grounded) action, which is of the form
//!     LHS := RHS
//! where both LHS (left-hand side) and RHS (right-hand side) are terms in our language,
//! with the particularity that LHS must be either a state variable or a fluent-headed nested term.
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
