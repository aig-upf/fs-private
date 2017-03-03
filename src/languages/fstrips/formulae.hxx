
#pragma once

#include <iostream>

#include <fs_types.hxx>
#include <utils/visitor.hxx>

namespace fs0 {
class State;
class ProblemInfo;
class Binding;
}

namespace fs0 { namespace language { namespace fstrips {

class Term;
class BoundVariable;
class AtomicFormula;
class Conjunction;
class ExistentiallyQuantifiedFormula;
class Tautology;
class Contradiction;

//! The base interface for a logic formula
class Formula :
// 	public Loki::BaseVisitable<>
 	public Loki::BaseVisitable<void, Loki::ThrowCatchAll, true>
//	public fs0::utils::BaseVisitable<void>
		
{
public:
	Formula() {}
	virtual ~Formula() {}

	//! Clone idiom
	virtual Formula* clone() const = 0;

	//! Return the boolean interpretation of the current formula under the given assignment and binding.
	virtual bool interpret(const PartialAssignment& assignment, const Binding& binding) const = 0;
	virtual bool interpret(const State& state, const Binding& binding) const = 0;
	virtual bool interpret(const PartialAssignment& assignment) const;
	virtual bool interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Formula& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;

	
	//! By default, formulae are not tautology nor contradiction
	virtual bool is_tautology() const { return false; }
	virtual bool is_contradiction() const { return false; }
};

//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class AtomicFormula : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	AtomicFormula(const std::vector<const Term*>& subterms) : _subterms(subterms), _interpreted_subterms(subterms.size(), -1) {}

	virtual ~AtomicFormula();

	//! Clone the type of formula assigning the given subterms
	virtual AtomicFormula* clone(const std::vector<const Term*>& subterms) const = 0;
	AtomicFormula* clone() const override;

	const std::vector<const Term*>& getSubterms() const { return _subterms; }

	bool interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	bool interpret(const State& state, const Binding& binding) const override;
	using Formula::interpret;

	//! Prints a representation of the object to the given stream.
// 	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;

	//! A helper to recursively evaluate the formula - must be subclassed
	virtual bool _satisfied(const ObjectIdxVector& values) const = 0;


protected:
	//! The formula subterms
	std::vector<const Term*> _subterms;

	//! The last interpretation of the subterms (acts as a cache)
	mutable std::vector<ObjectIdx> _interpreted_subterms;
};

class ExternallyDefinedFormula : public AtomicFormula {
public:
	ExternallyDefinedFormula(const std::vector<const Term*>& subterms) : AtomicFormula(subterms) {}
	
	virtual std::string name() const = 0;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};


class AxiomaticFormula : public AtomicFormula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	AxiomaticFormula(const std::vector<const Term*>& subterms) : AtomicFormula(subterms) {}
	
	//! To be subclassed
	virtual std::string name() const = 0;
	
	virtual AxiomaticFormula* clone(const std::vector<const Term*>& subterms) const override = 0;
	
	bool interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	bool interpret(const State& state, const Binding& binding) const override;
	
	//! To be subclassed
	virtual bool compute(const State& state, std::vector<ObjectIdx>& arguments) const = 0;
	bool _satisfied(const ObjectIdxVector& values) const override { throw std::runtime_error("This shouldn't be called"); };

	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};


//! The True truth value
class Tautology : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	Tautology* clone() const override { return new Tautology; }

	bool interpret(const PartialAssignment& assignment, const Binding& binding) const override { return true; }
	bool interpret(const State& state, const Binding& binding) const override { return true; }

	bool is_tautology() const override { return true; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override { os << "True"; return os; }
};

//! The False truth value
class Contradiction : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	Contradiction* clone() const override { return new Contradiction; }

	bool interpret(const PartialAssignment& assignment, const Binding& binding) const override { return false; }
	bool interpret(const State& state, const Binding& binding) const override { return false; }

	bool is_contradiction() const override { return true; }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override { os << "False"; return os; }
};

//! A logical conjunction
class Conjunction : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	Conjunction(const std::vector<const AtomicFormula*>& conjuncts) : _conjuncts(conjuncts) {}

	Conjunction(const Conjunction& conjunction);

	virtual ~Conjunction() {
		for (const auto ptr:_conjuncts) delete ptr;
	}

	Conjunction* clone() const override { return new Conjunction(*this); }

	const std::vector<const AtomicFormula*>& getConjuncts() const { return _conjuncts; }

	bool interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	bool interpret(const State& state, const Binding& binding) const override;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

protected:
	//! The formula subterms
	std::vector<const AtomicFormula*> _conjuncts;
};

//! A conjunctive formula made up of atoms X=x - geared towards optimizing the `interpret` method
class AtomConjunction : public Conjunction {
public:
	using AtomT = std::pair<VariableIdx, ObjectIdx>;

	AtomConjunction(const std::vector<const AtomicFormula*>& conjuncts, const std::vector<AtomT>& atoms)
		: Conjunction(conjuncts), _atoms(atoms) {}

	~AtomConjunction() = default;
	AtomConjunction(const AtomConjunction&) = default;
	AtomConjunction& operator=(const AtomConjunction&) = default;
	AtomConjunction(AtomConjunction&&) = default;
	AtomConjunction& operator=(AtomConjunction&&) = default;
	AtomConjunction* clone() const override {return new AtomConjunction(*this); }
	
	using Conjunction::interpret;
	bool interpret(const State& state, const Binding& binding) const override { return interpret(state); }
	bool interpret(const State& state) const override;

protected:
	//! Each pair (X,x) represents a conjunct X=x
	std::vector<AtomT> _atoms;
};

//! An atomic formula, implicitly understood to be static (fluent atoms are considered terms with Boolean codomain)
class ExistentiallyQuantifiedFormula : public Formula {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	ExistentiallyQuantifiedFormula(const std::vector<const BoundVariable*>& variables, const Conjunction* subformula) : _variables(variables), _subformula(subformula) {}

	virtual ~ExistentiallyQuantifiedFormula() {
		delete _subformula;
	}

	ExistentiallyQuantifiedFormula(const ExistentiallyQuantifiedFormula& other);

	ExistentiallyQuantifiedFormula* clone() const override { return new ExistentiallyQuantifiedFormula(*this); }

	const Conjunction* getSubformula() const { return _subformula; }
	
	const std::vector<const BoundVariable*> getVariables() const { return _variables; }

	bool interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	bool interpret(const State& state, const Binding& binding) const override;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;

protected:
	//! The binding IDs of the existentially quantified variables
	std::vector<const BoundVariable*> _variables;

	//! ATM we only allow quantification of conjunctions
	const Conjunction* _subformula;

	//! A naive recursive implementation of the interpretation routine
	template <typename T>
	bool interpret_rec(const T& assignment, const Binding& binding, unsigned i) const;
};


//! A formula of the form t_1 <op> t_2, where t_i are terms and <op> is a basic relational
//! operator such as =, !=, >, etc.
class RelationalFormula : public AtomicFormula {
public:
	enum class Symbol {EQ, NEQ, LT, LEQ, GT, GEQ};

	RelationalFormula(const std::vector<const Term*>& subterms) : AtomicFormula(subterms) {
		assert(subterms.size() == 2);
	}
	
	virtual RelationalFormula* clone(const std::vector<const Term*>& subterms) const override = 0;

	virtual Symbol symbol() const = 0;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;


	const static std::map<RelationalFormula::Symbol, std::string> symbol_to_string;
// 	const static std::map<std::string, RelationalFormula::Symbol> string_to_symbol;

	const Term* lhs() const { return _subterms[0]; }
	const Term* rhs() const { return _subterms[1]; }

protected:
	inline bool _satisfied(const ObjectIdxVector& values) const override { return _satisfied(values[0], values[1]); }
	virtual bool _satisfied(ObjectIdx o1, ObjectIdx o2) const = 0;
};

class EQAtomicFormula : public RelationalFormula {
public:
	EQAtomicFormula(const std::vector<const Term*>& subterms) : RelationalFormula(subterms) {}

	EQAtomicFormula* clone(const std::vector<const Term*>& subterms) const { return new EQAtomicFormula(subterms); }

	inline bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 == o2; }

	virtual Symbol symbol() const { return Symbol::EQ; }
};

class NEQAtomicFormula : public RelationalFormula {
public:
	NEQAtomicFormula(const std::vector<const Term*>& subterms) : RelationalFormula(subterms) {}

	NEQAtomicFormula* clone(const std::vector<const Term*>& subterms) const { return new NEQAtomicFormula(subterms); }

	inline bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 != o2; }

	Symbol symbol() const { return Symbol::NEQ; }
};

class LTAtomicFormula : public RelationalFormula {
public:
	LTAtomicFormula(const std::vector<const Term*>& subterms) : RelationalFormula(subterms) {}

	LTAtomicFormula* clone(const std::vector<const Term*>& subterms) const { return new LTAtomicFormula(subterms); }

	bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 < o2; }

	Symbol symbol() const { return Symbol::LT; }
};

class LEQAtomicFormula : public RelationalFormula {
public:
	LEQAtomicFormula(const std::vector<const Term*>& subterms) : RelationalFormula(subterms) {}

	LEQAtomicFormula* clone(const std::vector<const Term*>& subterms) const { return new LEQAtomicFormula(subterms); }

	inline bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 <= o2; }

	Symbol symbol() const { return Symbol::LEQ; }
};

class GTAtomicFormula : public RelationalFormula {
public:
	GTAtomicFormula(const std::vector<const Term*>& subterms) : RelationalFormula(subterms) {}

	GTAtomicFormula* clone(const std::vector<const Term*>& subterms) const { return new GTAtomicFormula(subterms); }

	inline bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 > o2; }

	Symbol symbol() const { return Symbol::GT; }
};

class GEQAtomicFormula : public RelationalFormula {
public:
	GEQAtomicFormula(const std::vector<const Term*>& subterms) : RelationalFormula(subterms) {}

	GEQAtomicFormula* clone(const std::vector<const Term*>& subterms) const { return new GEQAtomicFormula(subterms); }

	inline bool _satisfied(ObjectIdx o1, ObjectIdx o2) const { return o1 >= o2; }

	Symbol symbol() const { return Symbol::GEQ; }
};

} } } // namespaces
