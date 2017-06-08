
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
	LOKI_DEFINE_CONST_VISITABLE();
	
	AdditionTerm(const std::vector<const Term*>& subterms);
	
	AdditionTerm* clone() const override { return new AdditionTerm(*this); }
	
	object_id interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	object_id interpret(const State& state, const Binding& binding) const override;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that subtracts up the values of the two subterms
class SubtractionTerm : public ArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	SubtractionTerm(const std::vector<const Term*>& subterms);
	
	SubtractionTerm* clone() const override { return new SubtractionTerm(*this); }
	
	object_id interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	object_id interpret(const State& state, const Binding& binding) const override;
	
	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that multiplies the values of the two subterms
class MultiplicationTerm : public ArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();
	
	MultiplicationTerm(const std::vector<const Term*>& subterms);
	
	MultiplicationTerm* clone() const override { return new MultiplicationTerm(*this); }
	
	object_id interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	object_id interpret(const State& state, const Binding& binding) const override;
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class AlldiffFormula : public ExternallyDefinedFormula {
public:
	AlldiffFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	AlldiffFormula(const AlldiffFormula& formula);
	
	AlldiffFormula* clone() const override { return new AlldiffFormula(*this); }
	AlldiffFormula* clone(const std::vector<const Term*>& subterms) const override { return new AlldiffFormula(subterms); }

	std::string name() const override { return "alldiff"; }
	
protected:
	bool _satisfied(const std::vector<object_id>& values) const override;
};

class SumFormula : public ExternallyDefinedFormula {
public:
	SumFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	SumFormula(const SumFormula& formula);
	
	SumFormula* clone() const override { return new SumFormula(*this); }
	SumFormula* clone(const std::vector<const Term*>& subterms) const override { return new SumFormula(subterms); }
	
	std::string name() const override { return "sum"; }
	
protected:
	bool _satisfied(const std::vector<object_id>& values) const override;
};


class NValuesFormula : public ExternallyDefinedFormula {
public:
	NValuesFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	NValuesFormula(const NValuesFormula& formula);
	
	NValuesFormula* clone() const override { return new NValuesFormula(*this); }	
	NValuesFormula* clone(const std::vector<const Term*>& subterms) const override { return new NValuesFormula(subterms); }
	
	std::string name() const override { return "nvalues"; }
	
protected:
	bool _satisfied(const std::vector<object_id>& values) const override;
};


} } } // namespaces
