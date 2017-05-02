
#include <numeric>

#include <languages/fstrips/builtin.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>
#include <cmath>
#include <cfenv>

namespace fs0 { namespace language { namespace fstrips {


std::set<std::string> ArithmeticTermFactory::_builtin_terms = {
	"+", "-", "*", "/", "^", "sin", "cos", "sqrt", "tan", "asin", "acos", "atan", "min", "max", "exp"
};

std::set<std::string> ArithmeticTermFactory::_unary_terms = {
	"sin", "cos", "sqrt", "tan", "asin", "acos", "atan", "exp"
};


bool ArithmeticTermFactory::isBuiltinTerm(const std::string& symbol) {
	return _builtin_terms.find(symbol) != _builtin_terms.end();
}

bool ArithmeticTermFactory::isUnaryTerm( const std::string& symbol ) {
	return _unary_terms.find(symbol) != _unary_terms.end();
}


const StaticHeadedNestedTerm*
ArithmeticTermFactory::create(const std::string& symbol, const std::vector<const Term*>& subterms) {
	if (symbol == "+")      return new AdditionTerm(subterms);
	else if (symbol == "-") return new SubtractionTerm(subterms);
	else if (symbol == "*") return new MultiplicationTerm(subterms);
    else if (symbol == "/") return new DivisionTerm( subterms );
    else if (symbol == "^") return new PowerTerm( subterms);
    else if (symbol == "min") return new MinTerm( subterms);
    else if (symbol == "max") return new MaxTerm( subterms);
	else if (symbol == "sqrt") return new SqrtTerm( subterms);
	else if (symbol == "sin") return new SineTerm( subterms );
	else if (symbol == "cos") return new CosineTerm( subterms );
	else if (symbol == "tan") return new TangentTerm( subterms );
	else if (symbol == "asin") return new ArcSineTerm( subterms );
	else if (symbol == "acos") return new ArcCosineTerm( subterms );
	else if (symbol == "atan") return new ArcTangentTerm( subterms );
    else if (symbol == "exp") return new ArcTangentTerm( subterms );
	return nullptr;
}


UnaryArithmeticTerm::UnaryArithmeticTerm( const std::vector<const Term*>& subterms )
    : ArithmeticTerm( subterms ) {}

ObjectIdx UnaryArithmeticTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
    ObjectIdx value = _subterms[0]->interpret(assignment, binding);
    return boost::apply_visitor( _unary_interpreter, value );
}

ObjectIdx UnaryArithmeticTerm::interpret(const State& state, const Binding& binding) const {
    ObjectIdx value = _subterms[0]->interpret(state, binding);
    return boost::apply_visitor( _unary_interpreter, value );
}

BinaryArithmeticTerm::BinaryArithmeticTerm( const std::vector<const Term*>& subterms )
    : ArithmeticTerm( subterms ) {}

ObjectIdx BinaryArithmeticTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
    ObjectIdx lhs = _subterms[0]->interpret(assignment, binding);
    ObjectIdx rhs = _subterms[1]->interpret(assignment, binding);
    return boost::apply_visitor( _binary_interpreter, lhs, rhs );
}

ObjectIdx BinaryArithmeticTerm::interpret(const State& state, const Binding& binding) const {
    ObjectIdx lhs = _subterms[0]->interpret(state, binding);
    ObjectIdx rhs = _subterms[1]->interpret(state, binding);
    return boost::apply_visitor( _binary_interpreter, lhs, rhs );
}

AdditionTerm::AdditionTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
    if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("+(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return lhs + rhs; };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return lhs + rhs; };
}

std::ostream& AdditionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " + " << *_subterms[1] << ")";
	return os;
}

SubtractionTerm::SubtractionTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
    if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("-(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return lhs - rhs; };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return lhs - rhs; };
}

std::ostream& SubtractionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " - " << *_subterms[1] << ")";
	return os;
}

MultiplicationTerm::MultiplicationTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
    if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("*(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return lhs * rhs; };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return lhs * rhs; };
}

std::ostream& MultiplicationTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " * " << *_subterms[1] << ")";
	return os;
}

DivisionTerm::DivisionTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
    if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("/(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return lhs / rhs; };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return lhs / rhs; };
}

std::ostream& DivisionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " / " << *_subterms[1] << ")";
	return os;
}

PowerTerm::PowerTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
    if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("^(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return (float)std::pow(lhs,rhs); };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return (float)std::pow(lhs,rhs); };
}

std::ostream& PowerTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " ^ " << *_subterms[1] << ")";
	return os;
}

// Sqrt Term Implementation
SqrtTerm::SqrtTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// Square root is a unary arithmetic term
		throw std::runtime_error("sqrt(x) is a unary function!");
	}
    _unary_interpreter.int_handler = [](int v) { return std::sqrt((float)v); };
    _unary_interpreter.float_handler = [](float v) { return std::sqrt(v); };
}

std::ostream& SqrtTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "sqrt(" << *_subterms[0] << ")";
	return os;
}

// Sine Term Implementation
SineTerm::SineTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// sin(x) a unary arithmetic term
		throw std::runtime_error("sin(x) is a unary function");
	}
    _unary_interpreter.int_handler = [](int v) { return std::sin((float)v); };
    _unary_interpreter.float_handler = [](float v) { return std::sin(v); };
}

std::ostream& SineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "sin(" << *_subterms[0] << ")";
	return os;
}

// Cosine Term Implementation
CosineTerm::CosineTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// cos(x) a unary arithmetic term
		throw std::runtime_error("cos(x) is a unary function");
	}
    _unary_interpreter.int_handler = [](int v) { return std::cos((float)v); };
    _unary_interpreter.float_handler = [](float v) { return std::cos(v); };
}

std::ostream& CosineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "cos(" << *_subterms[0] << ")";
	return os;
}

// Tanget Term Implementation
TangentTerm::TangentTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("tan(x) is a unary function");
	}
    auto impl = [](float v) -> float {
        // Check for singularity
    	std::feclearexcept(FE_ALL_EXCEPT);
    	float res = (float)std::tan(v);
    	if(std::fetestexcept(FE_INVALID)) {
    		throw std::runtime_error("FE_INVALID thrown while evaluating tan(x)!");
    	}
    	if ( std::isnan(res)) {
    		throw std::runtime_error("Evaluating tan(x) resulted in a NaN result");
    	}
        return res;
    };

    _unary_interpreter.int_handler = [impl](int v) { return impl((float)v); };
    _unary_interpreter.float_handler = [impl](float v) { return impl(v); };

}

std::ostream& TangentTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "tan(" << *_subterms[0] << ")";
	return os;
}

// Inverse Trigonometric Functions

// Sine Term Implementation
ArcSineTerm::ArcSineTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// asin(x) a unary arithmetic term
		throw std::runtime_error("asin(x) is a unary function");
	}
    _unary_interpreter.int_handler = [](int v) { return std::asin((float)v); };
    _unary_interpreter.float_handler = [](float v) { return std::asin(v); };

}

std::ostream& ArcSineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "asin(" << *_subterms[0] << ")";
	return os;
}


// Cosine Term Implementation
ArcCosineTerm::ArcCosineTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// acos(x) a unary arithmetic term
		throw std::runtime_error("acos(x) is a unary function");
	}
    _unary_interpreter.int_handler = [](int v) { return std::acos((float)v); };
    _unary_interpreter.float_handler = [](float v) { return std::acos(v); };
}

std::ostream& ArcCosineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "acos(" << *_subterms[0] << ")";
	return os;
}

// Tanget Term Implementation
ArcTangentTerm::ArcTangentTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// atan(x) a unary arithmetic term
		throw std::runtime_error("atan(x) is a unary function");
	}
    auto impl = [](float v) -> float {
        // Check for singularity
    	std::feclearexcept(FE_ALL_EXCEPT);
    	float res = (float)std::atan(v);
    	if(std::fetestexcept(FE_INVALID)) {
    		throw std::runtime_error("FE_INVALID thrown while evaluating atan(x)!");
    	}
    	if ( std::isnan(res)) {
    		throw std::runtime_error("Evaluating tan(x) aresulted in a NaN result");
    	}
        return res;
    };

    _unary_interpreter.int_handler = [impl](int v) { return impl((float)v); };
    _unary_interpreter.float_handler = [impl](float v) { return impl(v); };

}

std::ostream& ArcTangentTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "atan(" << *_subterms[0] << ")";
	return os;
}

// Tanget Term Implementation
ExpTerm::ExpTerm(const std::vector<const Term*>& subterms)
	: UnaryArithmeticTerm(subterms) {
	if ( subterms.size() > 1 ) {
		// atan(x) a unary arithmetic term
		throw std::runtime_error("exp(x) is a unary function");
	}
    auto impl = [](float v) -> float {
        // Check for singularity
    	std::feclearexcept(FE_ALL_EXCEPT);
    	float res = (float)std::exp(v);
    	if(std::fetestexcept(FE_OVERFLOW)) {
    		throw std::runtime_error("FE_OVERFLOW thrown while evaluating exp(x)!");
    	}
    	if ( std::isnan(res)) {
    		throw std::runtime_error("Evaluating exp(x) aresulted in a NaN result");
    	}
        return res;
    };

    _unary_interpreter.int_handler = [impl](int v) { return impl((float)v); };
    _unary_interpreter.float_handler = [impl](float v) { return impl(v); };
}

std::ostream& ExpTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "exp(" << *_subterms[0] << ")";
	return os;
}

// Min Term Implementation
MinTerm::MinTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
	if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("min(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return std::min(lhs,rhs); };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return std::min(lhs,rhs); };
}

std::ostream& MinTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "min(" << *_subterms[0] << ", " << *_subterms[1] << ")";
	return os;
}

// Max Term Implementation
MaxTerm::MaxTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
	if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("max(x,y) is a binary function");
	}
    _binary_interpreter.int_handler = [](int lhs, int rhs) { return std::max(lhs,rhs); };
    _binary_interpreter.float_handler = [](float lhs, float rhs) { return std::max(lhs,rhs); };
}

std::ostream& MaxTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "max(" << *_subterms[0] << ", " << *_subterms[1] << ")";
	return os;
}

AlldiffFormula::AlldiffFormula(const AlldiffFormula& formula) : AlldiffFormula(Utils::clone(formula._subterms)) {}

bool AlldiffFormula::_satisfied(const ObjectIdxVector& values) const {
	std::set<ObjectIdx> distinct;
	for (ObjectIdx val:values) {
		auto res = distinct.insert(val);
		if (!res.second) return false; // We found a duplicate, hence the formula is false
	}
	return true;
}

SumFormula::SumFormula(const SumFormula& formula) : SumFormula(Utils::clone(formula._subterms)) {}

namespace sum_detail {
    class add :
        public boost::static_visitor<ObjectIdx> {
    public:

        ObjectIdx operator()( int lhs, int rhs ) const {
            return lhs + rhs;
        }

        ObjectIdx operator()( float lhs, float rhs ) const {
            return (int)lhs + (int)rhs;
        }

        ObjectIdx operator()( float lhs, int rhs ) const {
            return (int)lhs + rhs ;
        }

        ObjectIdx operator()( int lhs, float rhs ) const {
            return lhs + (int)rhs;
        }
    };

    class eq :
        public boost::static_visitor<bool> {
    public:

        bool operator()( int lhs, int rhs ) const {
            return lhs == rhs;
        }

        bool operator()( float lhs, float rhs ) const{
            return (int)lhs == (int)rhs;
        }

        bool operator()( float lhs, int rhs ) const {
            return (int)lhs == rhs ;
        }

        bool operator()( int lhs, float rhs ) const {
            return lhs == (int)rhs;
        }
    };
}
bool SumFormula::_satisfied(const ObjectIdxVector& values) const {
	// sum(x_1, ..., x_n) meaning x_1 + ... + x_{n-1} = x_n
    if ( values.size() < 3 ) {
		throw std::runtime_error("sum(x_1,...,x_{n-1}) = x_n needs to have n > 2");
    }
	assert(values.size() > 1);
	ObjectIdx expected_sum = values.back();
	ObjectIdx addends_sum = values[0];
    for ( unsigned i = 1; i < values.size() - 1; i++ ) {
        addends_sum = boost::apply_visitor( sum_detail::add(), addends_sum, values[i] );
    }

	return boost::apply_visitor( sum_detail::eq(), addends_sum, expected_sum );
}

NValuesFormula::NValuesFormula(const NValuesFormula& formula) : NValuesFormula(Utils::clone(formula._subterms)) {}

bool NValuesFormula::_satisfied(const ObjectIdxVector& values) const {
	// nvalues(x_1, ..., x_n) meaning there are exactly x_n different values among variables <x_1, ... x_{n-1}>
	assert(values.size() > 1);
	assert(boost::get<int>(values[values.size()-1]) > 0);
	std::set<ObjectIdx> distinct;
	for (unsigned i = 0; i < values.size() - 1; ++i) {
		distinct.insert(values[i]);
	}
	return distinct.size() == (std::size_t) boost::get<int>(values[values.size()-1]);
}

} } } // namespaces
