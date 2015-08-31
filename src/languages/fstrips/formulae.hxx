
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/terms.hxx>
#include <problem_info.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

//! An atomic formula, implicitly understood to be static (fluent formulae are considered terms with Boolean codomain)
class AtomicFormula : public LogicalElement {
public:
	typedef const AtomicFormula* cptr;
	
	AtomicFormula(const std::vector<Term::cptr>& subterms) : _subterms(subterms) {}
	
	virtual ~AtomicFormula() {
		for (const auto ptr:_subterms) delete ptr;
	}
	
	const std::vector<Term::cptr>& getSubterms() const { return _subterms; }
	
	bool interpret(const PartialAssignment& assignment) const;
	bool interpret(const State& state) const;
	
	unsigned nestedness() const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;
	
	std::vector<Term::cptr> flatten() const;
	
	//! By default, formulae are not tautology nor contradiction
	virtual bool is_tautology() const { return false; }
	virtual bool is_contradiction() const { return false; }
	
protected:
	//! The formula subterms
	std::vector<Term::cptr> _subterms;
	
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

} } } // namespaces
