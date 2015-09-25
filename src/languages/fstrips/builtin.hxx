
#pragma once

#include <languages/fstrips/language.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

class ArithmeticTermFactory {
public:
	static StaticHeadedNestedTerm::cptr create(const std::string& symbol, const std::vector<const Term*>& subterms);
	
	static bool isBuiltinTerm(const std::string& symbol) {
		return symbol == "+" || symbol == "-" || symbol == "*";
	}
};

//! A statically-headed term that performs some arithmetic operation to its two subterms
class ArithmeticTerm : public StaticHeadedNestedTerm {
public:
	typedef const ArithmeticTerm* cptr;
	
	ArithmeticTerm(const std::vector<Term::cptr>& subterms);
};

//! A statically-headed term that adds up the values of the two subterms
class AdditionTerm : public ArithmeticTerm {
public:
	typedef const AdditionTerm* cptr;
	
	AdditionTerm(const std::vector<Term::cptr>& subterms);
	
	AdditionTerm* clone() const { return new AdditionTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
	
	std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

//! A statically-headed term that subtracts up the values of the two subterms
class SubtractionTerm : public ArithmeticTerm {
public:
	typedef const SubtractionTerm* cptr;
	
	SubtractionTerm(const std::vector<Term::cptr>& subterms);
	
	SubtractionTerm* clone() const { return new SubtractionTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
	
	std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

//! A statically-headed term that multiplies the values of the two subterms
class MultiplicationTerm : public ArithmeticTerm {
public:
	typedef const MultiplicationTerm* cptr;
	
	MultiplicationTerm(const std::vector<Term::cptr>& subterms);
	
	MultiplicationTerm* clone() const { return new MultiplicationTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
	
	std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class ExternallyDefinedFormula : public AtomicFormula {
public:
	typedef const ExternallyDefinedFormula* cptr;
	
	ExternallyDefinedFormula(const std::vector<Term::cptr>& subterms) : AtomicFormula(subterms) {}
	
	virtual std::string name() const = 0;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class AlldiffFormula : public ExternallyDefinedFormula {
public:
	typedef const AlldiffFormula* cptr;
	
	AlldiffFormula(const std::vector<Term::cptr>& subterms) : ExternallyDefinedFormula(subterms) {}
	
	virtual std::string name() const { return "alldiff"; }
	
protected:
	bool _satisfied(const ObjectIdxVector& values) const;
};

class SumFormula : public ExternallyDefinedFormula {
public:
	typedef const SumFormula* cptr;
	
	SumFormula(const std::vector<Term::cptr>& subterms) : ExternallyDefinedFormula(subterms) {}
	
	virtual std::string name() const { return "sum"; }
	
protected:
	bool _satisfied(const ObjectIdxVector& values) const;
};

} } } // namespaces
