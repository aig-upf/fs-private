
#pragma once

#include <languages/fstrips/language.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

class ArithmeticTermFactory {
public:
	static const StaticHeadedNestedTerm* create(const std::string& symbol, const std::vector<const Term*>& subterms);
	
	static bool isBuiltinTerm(const std::string& symbol) {
		return symbol == "+" || symbol == "-" || symbol == "*";
	}
};

//! A statically-headed term that adds up the values of the two subterms
class AdditionTerm : public ArithmeticTerm {
public:
	AdditionTerm(const std::vector<const Term*>& subterms);
	
	AdditionTerm* clone() const { return new AdditionTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const;
	ObjectIdx interpret(const State& state, const Binding& binding) const;
	
	std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

//! A statically-headed term that subtracts up the values of the two subterms
class SubtractionTerm : public ArithmeticTerm {
public:
	SubtractionTerm(const std::vector<const Term*>& subterms);
	
	SubtractionTerm* clone() const { return new SubtractionTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const;
	ObjectIdx interpret(const State& state, const Binding& binding) const;
	
	std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

//! A statically-headed term that multiplies the values of the two subterms
class MultiplicationTerm : public ArithmeticTerm {
public:
	MultiplicationTerm(const std::vector<const Term*>& subterms);
	
	MultiplicationTerm* clone() const { return new MultiplicationTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const;
	ObjectIdx interpret(const State& state, const Binding& binding) const;
	
	std::pair<int, int> getBounds() const;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class ExternallyDefinedFormula : public AtomicFormula {
public:
	ExternallyDefinedFormula(const std::vector<const Term*>& subterms) : AtomicFormula(subterms) {}
	
	virtual std::string name() const = 0;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class AlldiffFormula : public ExternallyDefinedFormula {
public:
	AlldiffFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	AlldiffFormula(const AlldiffFormula& formula);
	
	AlldiffFormula* clone() const { return new AlldiffFormula(*this); }
	AlldiffFormula* clone(const std::vector<const Term*>& subterms) const { return new AlldiffFormula(subterms); }

	std::string name() const { return "alldiff"; }
	
protected:
	bool _satisfied(const ObjectIdxVector& values) const;
};

class SumFormula : public ExternallyDefinedFormula {
public:
	SumFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	SumFormula(const SumFormula& formula);
	
	SumFormula* clone() const { return new SumFormula(*this); }
	SumFormula* clone(const std::vector<const Term*>& subterms) const { return new SumFormula(subterms); }
	
	std::string name() const { return "sum"; }
	
protected:
	bool _satisfied(const ObjectIdxVector& values) const;
};


class NValuesFormula : public ExternallyDefinedFormula {
public:
	NValuesFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	NValuesFormula(const NValuesFormula& formula);
	
	NValuesFormula* clone() const { return new NValuesFormula(*this); }	
	NValuesFormula* clone(const std::vector<const Term*>& subterms) const { return new NValuesFormula(subterms); }
	
	std::string name() const { return "nvalues"; }
	
protected:
	bool _satisfied(const ObjectIdxVector& values) const;
};


} } } // namespaces
