
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/terms.hxx>
#include <utils/binding.hxx>

namespace fs0 { class State; class ProblemInfo; }

namespace fs0 { namespace language { namespace fstrips {

class AtomicFormula; class Conjunction; class ExistentiallyQuantifiedFormula; class Tautology; class Contradiction;

//! The base interface for a logic formula
class Formula {
public:
	typedef const Formula* cptr;
	
	Formula() {}
	virtual ~Formula() {}
	
	//! Clone idiom
	virtual Formula* clone() const = 0;
	
	//! Processes a formula possibly containing bound variables and non-consolidated state variables,
	//! consolidating all possible state variables and performing the bindings according to the given variable binding
	virtual Formula::cptr bind(const Binding& binding, const ProblemInfo& info) const = 0;
	
	//! Return the boolean interpretation of the current formula under the given assignment and binding.
	virtual bool interpret(const PartialAssignment& assignment, const Binding& binding) const = 0;
	virtual bool interpret(const State& state, const Binding& binding) const = 0;
	bool interpret(const PartialAssignment& assignment) const { return interpret(assignment, Binding()); }
	bool interpret(const State& state) const  { return interpret(state, Binding()); }
	
	//! The level of nestedness of the formula
	virtual unsigned nestedness() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Formula& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;
	
	//! Returns a vector with all the terms involved in the current formula
	std::vector<Term::cptr> all_terms() const;
	
	//! Returns a vector with all the subformulae involved in the current formula
	virtual std::vector<const Formula*> all_formulae() const = 0;
	
	//! A small helper - returns a vector with all the atomic formulae involved in the current formula
	std::vector<const AtomicFormula*> all_atoms() const;
	
	//! By default, formulae are not tautology nor contradiction
	virtual bool is_tautology() const { return false; }
	virtual bool is_contradiction() const { return false; }
	
	//! Logical operations - ugly, but simple
	virtual Formula* conjunction(const Formula* 						other) const = 0;
	virtual Formula* conjunction(const AtomicFormula* 					other) const = 0;
	virtual Formula* conjunction(const Conjunction* 					other) const = 0;
	virtual Formula* conjunction(const ExistentiallyQuantifiedFormula*	other) const = 0;
	virtual Formula* conjunction(const Tautology* 						other) const { return clone(); }
	virtual Formula* conjunction(const Contradiction* 					other) const;
};

//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class AtomicFormula : public Formula {
public:
	typedef const AtomicFormula* cptr;
	
	AtomicFormula(const std::vector<Term::cptr>& subterms) : _subterms(subterms) {}
	
	virtual ~AtomicFormula() {
		for (const auto ptr:_subterms) delete ptr;
	}
	
	//! Clone the type of formula assigning the given subterms
	virtual AtomicFormula* clone(const std::vector<Term::cptr>& subterms) const = 0;
	AtomicFormula* clone() const;
	
	Formula::cptr bind(const fs0::Binding& binding, const fs0::ProblemInfo& info) const;

	const std::vector<Term::cptr>& getSubterms() const { return _subterms; }
	
	bool interpret(const PartialAssignment& assignment, const Binding& binding) const;
	bool interpret(const State& state, const Binding& binding) const;
	using Formula::interpret;
	
	unsigned nestedness() const;
	
	std::vector<const Formula*> all_formulae() const { return std::vector<Formula::cptr>(1, this); }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;
	
	std::vector<Term::cptr> all_terms() const;
	
	virtual Formula* conjunction(const Formula* 							other) const { return other->conjunction(this); }
	virtual Formula* conjunction(const AtomicFormula* 						other) const { throw std::runtime_error("Unimplemented"); }
	virtual Formula* conjunction(const Conjunction* 						other) const { throw std::runtime_error("Unimplemented"); }
	virtual Formula* conjunction(const ExistentiallyQuantifiedFormula*		other) const { throw std::runtime_error("Unimplemented"); }
	
protected:
	//! The formula subterms
	std::vector<Term::cptr> _subterms;
	
	//! A helper to recursively evaluate the formula - must be subclassed
	virtual bool _satisfied(const ObjectIdxVector& values) const = 0;
};


//! The True truth value
class Tautology : public Formula {
public:
	typedef const Tautology* cptr;
	
	Tautology* bind(const Binding& binding, const ProblemInfo& info) const { return new Tautology; }
	Tautology* clone() const { return new Tautology; }
	
	unsigned nestedness() const { return 0; }
	
	std::vector<const Formula*> all_formulae() const { return std::vector<Formula::cptr>(1, this); }
	
	bool interpret(const PartialAssignment& assignment, const Binding& binding) const { return true; }
	bool interpret(const State& state, const Binding& binding) const { return true; }
	
	bool is_tautology() const { return true; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const { os << "True"; return os; }
	
	Formula* conjunction(const Formula* 							other) const;
	Formula* conjunction(const AtomicFormula* 						other) const;
	Formula* conjunction(const Conjunction* 						other) const;
	Formula* conjunction(const ExistentiallyQuantifiedFormula*		other) const;
};

//! The False truth value
class Contradiction : public Formula {
public:
	typedef const Contradiction* cptr;
	
	
	Contradiction* bind(const Binding& binding, const ProblemInfo& info) const { return new Contradiction; }
	Contradiction* clone() const { return new Contradiction; }
	
	unsigned nestedness() const { return 0; }
	
	std::vector<const Formula*> all_formulae() const { return std::vector<Formula::cptr>(1, this); }
	
	bool interpret(const PartialAssignment& assignment, const Binding& binding) const { return false; }
	bool interpret(const State& state, const Binding& binding) const { return false; }
	
	bool is_contradiction() const { return true; }
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const { os << "False"; return os; }

	Formula* conjunction(const Formula* 							other) const;
	Formula* conjunction(const AtomicFormula* 						other) const;
	Formula* conjunction(const Conjunction* 						other) const;
	Formula* conjunction(const ExistentiallyQuantifiedFormula*		other) const;
};

//! A logical conjunction
class Conjunction : public Formula {
public:
	friend class LogicalOperations;
	
	typedef const Conjunction* cptr;
	
	Conjunction(const std::vector<AtomicFormula::cptr>& conjuncts) : _conjuncts(conjuncts) {}
	
	Conjunction(const Conjunction& conjunction) {
		for (const AtomicFormula::cptr conjunct:conjunction._conjuncts) {
			_conjuncts.push_back(conjunct->clone());
		}
	}
	
	virtual ~Conjunction() {
		for (const auto ptr:_conjuncts) delete ptr;
	}
	
	Conjunction* clone() const { return new Conjunction(*this); }
	
	Formula::cptr bind(const Binding& binding, const fs0::ProblemInfo& info) const;
	
	const std::vector<AtomicFormula::cptr>& getConjuncts() const { return _conjuncts; }
	
	bool interpret(const PartialAssignment& assignment, const Binding& binding) const;
	bool interpret(const State& state, const Binding& binding) const;
	
	unsigned nestedness() const;
	
	std::vector<const Formula*> all_formulae() const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	Formula* conjunction(const fs0::language::fstrips::Formula* other) const;
	Formula* conjunction(const fs0::language::fstrips::AtomicFormula* other) const;
	Conjunction* conjunction(const Conjunction* 						other) const;
	Formula* conjunction(const fs0::language::fstrips::ExistentiallyQuantifiedFormula* other) const;
	
protected:
	//! The formula subterms
	std::vector<AtomicFormula::cptr> _conjuncts;
};


//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class ExistentiallyQuantifiedFormula : public Formula {
public:
	friend class LogicalOperations;
	
	typedef const ExistentiallyQuantifiedFormula* cptr;
	
	ExistentiallyQuantifiedFormula(const std::vector<BoundVariable>& variables, Conjunction::cptr subformula) : _variables(variables), _subformula(subformula) {}
	
	virtual ~ExistentiallyQuantifiedFormula() {
		delete _subformula;
	}
	
	ExistentiallyQuantifiedFormula(const ExistentiallyQuantifiedFormula& formula) 
		: _variables(formula._variables), _subformula(formula._subformula->clone()) {}
	
	ExistentiallyQuantifiedFormula* clone() const { return new ExistentiallyQuantifiedFormula(*this); }
	
	Formula::cptr bind(const Binding& binding, const fs0::ProblemInfo& info) const;
	
	const Conjunction::cptr getSubformula() const { return _subformula; }
	
	bool interpret(const PartialAssignment& assignment, const Binding& binding) const;
	bool interpret(const State& state, const Binding& binding) const;
	
	unsigned nestedness() const { return _subformula->nestedness(); }
	
	std::vector<const Formula*> all_formulae() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	Formula* conjunction(const Formula* 							other) const;
	Formula* conjunction(const AtomicFormula* 						other) const;
	Formula* conjunction(const Conjunction* 						other) const;
	Formula* conjunction(const ExistentiallyQuantifiedFormula*		other) const;
	
protected:
	//! The binding IDs of the existentially quantified variables
	std::vector<BoundVariable> _variables;
	
	//! ATM we only allow quantification of conjunctions
	Conjunction::cptr _subformula;
	
	//! A naive recursive implementation of the interpretation routine
	template <typename T>
	bool interpret_rec(const T& assignment, const Binding& binding, unsigned i) const;
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

	virtual RelationalFormula* clone(const std::vector<Term::cptr>& subterms) const = 0;
	
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
	
	EQAtomicFormula* clone(const std::vector<Term::cptr>& subterms) const { return new EQAtomicFormula(subterms); }
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 == o2; }
	
	virtual Symbol symbol() const { return Symbol::EQ; }
};

class NEQAtomicFormula : public RelationalFormula {
public:
	NEQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
	
	NEQAtomicFormula* clone(const std::vector<Term::cptr>& subterms) const { return new NEQAtomicFormula(subterms); }
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }
	
	Symbol symbol() const { return Symbol::NEQ; }
};

class LTAtomicFormula : public RelationalFormula {
public:
	LTAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
	
	LTAtomicFormula* clone(const std::vector<Term::cptr>& subterms) const { return new LTAtomicFormula(subterms); }
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }
	
	Symbol symbol() const { return Symbol::LT; }
};

class LEQAtomicFormula : public RelationalFormula {
public:
	LEQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
	
	LEQAtomicFormula* clone(const std::vector<Term::cptr>& subterms) const { return new LEQAtomicFormula(subterms); }
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }
	
	Symbol symbol() const { return Symbol::LEQ; }
};

class GTAtomicFormula : public RelationalFormula {
public:
	GTAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
	
	GTAtomicFormula* clone(const std::vector<Term::cptr>& subterms) const { return new GTAtomicFormula(subterms); }
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 > o2; }
	
	Symbol symbol() const { return Symbol::GT; }
};

class GEQAtomicFormula : public RelationalFormula {
public:
	GEQAtomicFormula(const std::vector<Term::cptr>& subterms) : RelationalFormula(subterms) {}
	
	GEQAtomicFormula* clone(const std::vector<Term::cptr>& subterms) const { return new GEQAtomicFormula(subterms); }
		
	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 >= o2; }
	
	Symbol symbol() const { return Symbol::GEQ; }
};

} } } // namespaces
