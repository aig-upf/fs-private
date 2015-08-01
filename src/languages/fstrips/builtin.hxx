
#pragma once

#include <languages/fstrips/language.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

class BuiltinTermFactory {
public:
	static StaticHeadedNestedTerm::cptr create(const std::string& symbol, const std::vector<const Term*>& subterms);
	
	static bool isBuiltinTerm(const std::string& symbol) {
		return symbol == "+" || symbol == "-" || symbol == "*";
	}
};


//! A statically-headed term that adds up the values of the two subterms
class AdditionTerm : public StaticHeadedNestedTerm {
public:
	typedef const AdditionTerm* cptr;
	
	AdditionTerm(const std::vector<Term::cptr>& subterms);
	
	AdditionTerm* clone() const { return new AdditionTerm(*this); }
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
};

//! A statically-headed term that subtracts up the values of the two subterms
class SubtractionTerm : public StaticHeadedNestedTerm {
public:
	typedef const SubtractionTerm* cptr;
	
	SubtractionTerm(const std::vector<Term::cptr>& subterms);
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	SubtractionTerm* clone() const { return new SubtractionTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
};

//! A statically-headed term that multiplies the values of the two subterms
class MultiplicationTerm : public StaticHeadedNestedTerm {
public:
	typedef const MultiplicationTerm* cptr;
	
	MultiplicationTerm(const std::vector<Term::cptr>& subterms);
	
	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
	
	MultiplicationTerm* clone() const { return new MultiplicationTerm(*this); }
	
	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;
};

} } } // namespaces
