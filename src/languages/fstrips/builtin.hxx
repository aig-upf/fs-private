
#pragma once

#include <languages/fstrips/language.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

class ArithmeticTermFactory {
public:
	static const StaticHeadedNestedTerm* create(const std::string& symbol, const std::vector<const Term*>& subterms, ObjectType value_type );

    static bool isBuiltinTerm(const std::string& symbol);
    static bool isUnaryTerm( const std::string& symbol );
private:

    static std::set< std::string > _builtin_terms;
    static std::set< std::string > _unary_terms;
};

//! A statically-headed term that adds up the values of the two subterms
class AdditionTerm : public ArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	AdditionTerm(const std::vector<const Term*>& subterms, ObjectType value_type);

	AdditionTerm* clone() const override { return new AdditionTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	ObjectIdx interpret(const State& state, const Binding& binding) const override;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that subtracts up the values of the two subterms
class SubtractionTerm : public ArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	SubtractionTerm(const std::vector<const Term*>& subterms, ObjectType value_type);

	SubtractionTerm* clone() const override { return new SubtractionTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	ObjectIdx interpret(const State& state, const Binding& binding) const override;

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};



//! A statically-headed term that multiplies the values of the two subterms
class MultiplicationTerm : public ArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	MultiplicationTerm(const std::vector<const Term*>& subterms, ObjectType value_type);

	MultiplicationTerm* clone() const override { return new MultiplicationTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	ObjectIdx interpret(const State& state, const Binding& binding) const override;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that divides the values of the two subterms
class DivisionTerm : public ArithmeticTerm {
public:
	typedef const DivisionTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	DivisionTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	DivisionTerm* clone() const override { return new DivisionTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

//! A statically-headed term that divides the values of the two subterms
class PowerTerm : public ArithmeticTerm {
public:
	typedef const PowerTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	PowerTerm(const std::vector<const Term*>& subterms, ObjectType value_type);

	PowerTerm* clone() const { return new PowerTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class SqrtTerm : public ArithmeticTerm {
public:
	typedef const SqrtTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	SqrtTerm(const std::vector<const Term*>& subterms, ObjectType value_type);

	SqrtTerm* clone() const { return new SqrtTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class SineTerm : public ArithmeticTerm {
public:
	typedef const SineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	SineTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	SineTerm* clone() const { return new SineTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class CosineTerm : public ArithmeticTerm {
public:
	typedef const CosineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	CosineTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	CosineTerm* clone() const { return new CosineTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class TangentTerm : public ArithmeticTerm {
public:
	typedef const TangentTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	TangentTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	TangentTerm* clone() const { return new TangentTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class ArcSineTerm : public ArithmeticTerm {
public:
	typedef const ArcSineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ArcSineTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	ArcSineTerm* clone() const { return new ArcSineTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class ArcCosineTerm : public ArithmeticTerm {
public:
	typedef const ArcCosineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ArcCosineTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	ArcCosineTerm* clone() const { return new ArcCosineTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class ArcTangentTerm : public ArithmeticTerm {
public:
	typedef const ArcTangentTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ArcTangentTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	ArcTangentTerm* clone() const { return new ArcTangentTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};


class ExpTerm : public ArithmeticTerm {
public:
	typedef const ExpTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ExpTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	ExpTerm* clone() const { return new ExpTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const;
};

class MinTerm : public ArithmeticTerm {
public:
	typedef const MinTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	MinTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	MinTerm* clone() const override { return new ArcTangentTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class MaxTerm : public ArithmeticTerm {
public:
	typedef const MaxTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	MaxTerm(const std::vector<const Term*>& subterms, ObjectType value_type);
	MaxTerm* clone() const override { return new ArcTangentTerm(*this); }

	ObjectIdx interpret(const PartialAssignment& assignment) const;
	ObjectIdx interpret(const State& state) const;

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
	bool _satisfied(const ObjectIdxVector& values) const override;
};

class SumFormula : public ExternallyDefinedFormula {
public:
	SumFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	SumFormula(const SumFormula& formula);

	SumFormula* clone() const override { return new SumFormula(*this); }
	SumFormula* clone(const std::vector<const Term*>& subterms) const override { return new SumFormula(subterms); }

	std::string name() const override { return "sum"; }

protected:
	bool _satisfied(const ObjectIdxVector& values) const override;
};


class NValuesFormula : public ExternallyDefinedFormula {
public:
	NValuesFormula(const std::vector<const Term*>& subterms) : ExternallyDefinedFormula(subterms) {}
	NValuesFormula(const NValuesFormula& formula);

	NValuesFormula* clone() const override { return new NValuesFormula(*this); }
	NValuesFormula* clone(const std::vector<const Term*>& subterms) const override { return new NValuesFormula(subterms); }

	std::string name() const override { return "nvalues"; }

protected:
	bool _satisfied(const ObjectIdxVector& values) const override;
};


} } } // namespaces
