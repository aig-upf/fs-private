
#pragma once

#include <languages/fstrips/language.hxx>
#include <fs_types.hxx>

namespace fs0 { class State; }

namespace fs0 { namespace language { namespace fstrips {

using ObjectIdx = fs0::ObjectIdx;

class ArithmeticTermFactory {
public:
	static const StaticHeadedNestedTerm* create(const std::string& symbol, const std::vector<const Term*>& subterms );

    static bool isBuiltinTerm(const std::string& symbol);
    static bool isUnaryTerm( const std::string& symbol );
private:

    static std::set< std::string > _builtin_terms;
    static std::set< std::string > _unary_terms;
};

class UnaryArithmeticTerm : public ArithmeticTerm {
protected:
    class unary_term_interpreter :
        public boost::static_visitor<ObjectIdx> {
    public:
        std::function<ObjectIdx(int)>     int_handler;
        std::function<ObjectIdx(float)>   float_handler;

        ObjectIdx operator()( int v ) const  {
            return int_handler(v);
        }

        ObjectIdx operator()( float v ) const {
            return float_handler(v);
        }
    };
    unary_term_interpreter _unary_interpreter;
public:
    UnaryArithmeticTerm(const std::vector<const Term*>& subterms);

	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	ObjectIdx interpret(const State& state, const Binding& binding) const override;

};

class BinaryArithmeticTerm : public ArithmeticTerm {
protected:
    class binary_term_interpreter :
        public boost::static_visitor<ObjectIdx> {
    public:

        std::function<ObjectIdx(int,int)>   int_handler;
        std::function<ObjectIdx(float,float)>   float_handler;

        ObjectIdx operator()( int lhs, int rhs ) const {
            return int_handler(lhs, rhs);
        }

        ObjectIdx operator()( float lhs, float rhs ) const {
            return float_handler(lhs, rhs);
        }

        ObjectIdx operator()( float lhs, int rhs ) const {
            float converted_rhs = rhs;
            return operator()(lhs, converted_rhs);
        }

        ObjectIdx operator()( int lhs, float rhs ) const {
            float converted_lhs = lhs;
            return operator()(converted_lhs, rhs);
        }
    };
    binary_term_interpreter _binary_interpreter;

public:
    BinaryArithmeticTerm(const std::vector<const Term*>& subterms);

	ObjectIdx interpret(const PartialAssignment& assignment, const Binding& binding) const override;
	ObjectIdx interpret(const State& state, const Binding& binding) const override;

};

//! A statically-headed term that adds up the values of the two subterms
class AdditionTerm : public BinaryArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	AdditionTerm(const std::vector<const Term*>& subterms);

	AdditionTerm* clone() const override { return new AdditionTerm(*this); }


	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that subtracts up the values of the two subterms
class SubtractionTerm : public BinaryArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	SubtractionTerm(const std::vector<const Term*>& subterms);

	SubtractionTerm* clone() const override { return new SubtractionTerm(*this); }

	//! Prints a representation of the object to the given stream.
	std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};



//! A statically-headed term that multiplies the values of the two subterms
class MultiplicationTerm : public BinaryArithmeticTerm {
public:
	LOKI_DEFINE_CONST_VISITABLE();

	MultiplicationTerm(const std::vector<const Term*>& subterms);

	MultiplicationTerm* clone() const override { return new MultiplicationTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that divides the values of the two subterms
class DivisionTerm : public BinaryArithmeticTerm {
public:
	typedef const DivisionTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	DivisionTerm(const std::vector<const Term*>& subterms);
	DivisionTerm* clone() const override { return new DivisionTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

//! A statically-headed term that divides the values of the two subterms
class PowerTerm : public BinaryArithmeticTerm {
public:
	typedef const PowerTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	PowerTerm(const std::vector<const Term*>& subterms);

	PowerTerm* clone() const override{ return new PowerTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class SqrtTerm : public UnaryArithmeticTerm {
public:
	typedef const SqrtTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	SqrtTerm(const std::vector<const Term*>& subterms);
	SqrtTerm* clone() const override { return new SqrtTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class SineTerm : public UnaryArithmeticTerm {
public:
	typedef const SineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	SineTerm(const std::vector<const Term*>& subterms);
	SineTerm* clone() const override { return new SineTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class CosineTerm : public UnaryArithmeticTerm {
public:
	typedef const CosineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	CosineTerm(const std::vector<const Term*>& subterms);
	CosineTerm* clone() const override { return new CosineTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class TangentTerm : public UnaryArithmeticTerm {
public:
	typedef const TangentTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	TangentTerm(const std::vector<const Term*>& subterms);
	TangentTerm* clone() const override { return new TangentTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class ArcSineTerm : public UnaryArithmeticTerm {
public:
	typedef const ArcSineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ArcSineTerm(const std::vector<const Term*>& subterms);
	ArcSineTerm* clone() const override { return new ArcSineTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class ArcCosineTerm : public UnaryArithmeticTerm {
public:
	typedef const ArcCosineTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ArcCosineTerm(const std::vector<const Term*>& subterms);
	ArcCosineTerm* clone() const override { return new ArcCosineTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class ArcTangentTerm : public UnaryArithmeticTerm {
public:
	typedef const ArcTangentTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ArcTangentTerm(const std::vector<const Term*>& subterms);
	ArcTangentTerm* clone() const override { return new ArcTangentTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};


class ExpTerm : public UnaryArithmeticTerm {
public:
	typedef const ExpTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	ExpTerm(const std::vector<const Term*>& subterms);
	ExpTerm* clone() const override { return new ExpTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class MinTerm : public BinaryArithmeticTerm {
public:
	typedef const MinTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	MinTerm(const std::vector<const Term*>& subterms);
	MinTerm* clone() const override { return new MinTerm(*this); }

	//! Prints a representation of the object to the given stream.
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override;
};

class MaxTerm : public BinaryArithmeticTerm {
public:
	typedef const MaxTerm* cptr;
    LOKI_DEFINE_CONST_VISITABLE();

	MaxTerm(const std::vector<const Term*>& subterms);
	MaxTerm* clone() const override { return new MaxTerm(*this); }

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
