
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
    else if (symbol == "exp") return new ExpTerm( subterms );
	return nullptr;
}


UnaryArithmeticTerm::UnaryArithmeticTerm( const std::vector<const Term*>& subterms )
    : ArithmeticTerm( subterms ) {}

object_id UnaryArithmeticTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
    object_id value = _subterms[0]->interpret(assignment, binding);
	if (o_type(value) == type_id::int_t)
		return _int_handler(fs0::value<int>(value));
	if (o_type(value) == type_id::float_t)
		return _float_handler(fs0::value<float>(value));
	throw std::runtime_error("Unary Arithmetic Term::interpret Unsupported type: " + to_string(o_type(value)));
	return object_id::INVALID;

}

object_id UnaryArithmeticTerm::interpret(const State& state, const Binding& binding) const {
    object_id value = _subterms[0]->interpret(state, binding);
	if (o_type(value) == type_id::int_t)
		return _int_handler(fs0::value<int>(value));
	if (o_type(value) == type_id::float_t)
		return _float_handler(fs0::value<float>(value));
	throw std::runtime_error("Unary Arithmetic Term::interpret Unsupported type: " + to_string(o_type(value)));
	return object_id::INVALID;
}

BinaryArithmeticTerm::BinaryArithmeticTerm( const std::vector<const Term*>& subterms )
    : ArithmeticTerm( subterms ) {}

object_id BinaryArithmeticTerm::interpret(const PartialAssignment& assignment, const Binding& binding) const {
    object_id lhs = _subterms[0]->interpret(assignment, binding);
    object_id rhs = _subterms[1]->interpret(assignment, binding);

	if (o_type(lhs) == type_id::int_t ) {
		if ( o_type(lhs) == o_type(rhs) )
			return _int_handler(fs0::value<int>(lhs), fs0::value<int>(rhs));
		//! rhs is a float, promote lhs to float
		int unwrapped_lhs = fs0::value<int>(lhs);
		return _float_handler( (float)unwrapped_lhs, fs0::value<float>(rhs));
	}

	if ( o_type(lhs) == type_id::float_t ) {
		if ( o_type(lhs) == o_type(rhs) )
			return _float_handler(fs0::value<float>(lhs), fs0::value<float>(rhs));
		//! rhs is an int, promote it to float
		int unwrapped_rhs = fs0::value<int>(rhs);
		return _float_handler( fs0::value<float>(lhs), (float)unwrapped_rhs);
	}

	throw std::runtime_error(	"Binary Arithmetic Term::interpret Unsupported combination of types: "
								+ to_string(o_type(lhs))
								+ " and "
								+ to_string(o_type(rhs))
							);
	return object_id::INVALID;
}

object_id BinaryArithmeticTerm::interpret(const State& state, const Binding& binding) const {
    object_id lhs = _subterms[0]->interpret(state, binding);
    object_id rhs = _subterms[1]->interpret(state, binding);
	if (o_type(lhs) == type_id::int_t ) {
		if ( o_type(lhs) == o_type(rhs) )
			return _int_handler(fs0::value<int>(lhs), fs0::value<int>(rhs));
		//! rhs is a float, promote lhs to float
		int unwrapped_lhs = fs0::value<int>(lhs);
		return _float_handler( (float)unwrapped_lhs, fs0::value<float>(rhs));
	}

	if ( o_type(lhs) == type_id::float_t ) {
		if ( o_type(lhs) == o_type(rhs) )
			return _float_handler(fs0::value<float>(lhs), fs0::value<float>(rhs));
		//! rhs is an int, promote it to float
		int unwrapped_rhs = fs0::value<int>(rhs);
		return _float_handler( fs0::value<float>(lhs), (float)unwrapped_rhs);
	}

	throw std::runtime_error(	"Binary Arithmetic Term::interpret Unsupported combination of types: "
								+ to_string(o_type(lhs))
								+ " and "
								+ to_string(o_type(rhs))
							);
	return object_id::INVALID;
}

AdditionTerm::AdditionTerm(const std::vector<const Term*>& subterms)
	: BinaryArithmeticTerm(subterms) {
    if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("+(x,y) is a binary function");
	}
    _int_handler = [](int lhs, int rhs) { return make_object(lhs + rhs); };
    _float_handler = [](float lhs, float rhs) { return make_object(lhs + rhs); };
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
    _int_handler = [](int lhs, int rhs) { return make_object(lhs - rhs); };
    _float_handler = [](float lhs, float rhs) { return make_object(lhs - rhs); };
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
    _int_handler = [](int lhs, int rhs) { return make_object(lhs * rhs); };
    _float_handler = [](float lhs, float rhs) { return make_object(lhs * rhs); };
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
    _int_handler = [](int lhs, int rhs) { return make_object(lhs / rhs); };
    _float_handler = [](float lhs, float rhs) { return make_object(lhs / rhs); };
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
    _int_handler = [](int lhs, int rhs) { return make_object((float)std::pow(lhs,rhs)); };
    _float_handler = [](float lhs, float rhs) { return make_object((float)std::pow(lhs,rhs)); };
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
    _int_handler = [](int v) { return make_object(std::sqrt((float)v)); };
    _float_handler = [](float v) { return make_object(std::sqrt(v)); };
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
    _int_handler = [](int v) { return make_object(std::sin((float)v)); };
    _float_handler = [](float v) { return make_object(std::sin(v)); };
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
    _int_handler = [](int v) { return make_object(std::cos((float)v)); };
    _float_handler = [](float v) { return make_object(std::cos(v)); };
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

    _int_handler = [impl](int v) { return make_object(impl((float)v)); };
    _float_handler = [impl](float v) { return make_object(impl(v)); };

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
    _int_handler = [](int v) { return make_object(std::asin((float)v)); };
    _float_handler = [](float v) { return make_object(std::asin(v)); };

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
    _int_handler = [](int v) { return make_object(std::acos((float)v)); };
    _float_handler = [](float v) { return make_object(std::acos(v)); };
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

    _int_handler = [impl](int v) { return make_object(impl((float)v)); };
    _float_handler = [impl](float v) { return make_object(impl(v)); };

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

    _int_handler = [impl](int v) { return make_object(impl((float)v)); };
    _float_handler = [impl](float v) { return make_object(impl(v)); };
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
    _int_handler = [](int lhs, int rhs) { return make_object(std::min(lhs,rhs)); };
    _float_handler = [](float lhs, float rhs) { return make_object(std::min(lhs,rhs)); };
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
    _int_handler = [](int lhs, int rhs) { return make_object(std::max(lhs,rhs)); };
    _float_handler = [](float lhs, float rhs) { return make_object(std::max(lhs,rhs)); };
}

std::ostream& MaxTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "max(" << *_subterms[0] << ", " << *_subterms[1] << ")";
	return os;
}

AlldiffFormula::AlldiffFormula(const AlldiffFormula& formula) : AlldiffFormula(Utils::clone(formula._subterms)) {}

bool AlldiffFormula::_satisfied(const std::vector<object_id>& values) const {
	std::set<object_id> distinct;
	for (object_id val:values) {
		auto res = distinct.insert(val);
		if (!res.second) return false; // We found a duplicate, hence the formula is false
	}
	return true;
}

SumFormula::SumFormula(const SumFormula& formula) : SumFormula(Utils::clone(formula._subterms)) {}

namespace sum_detail {

	object_id add( object_id lhs, object_id rhs ) {
		if ( o_type(lhs) == type_id::int_t ) {
			if ( o_type(lhs) == o_type(rhs) )
				return make_object( fs0::value<int>(lhs) + fs0::value<int>(rhs));
			return make_object( fs0::value<int>(lhs) + (int)fs0::value<float>(rhs));
		}
		if ( o_type(lhs) == o_type(rhs)) {
			return make_object( (int)fs0::value<float>(lhs) + (int)fs0::value<float>(rhs));
		}
		return make_object( fs0::value<int>(rhs) + (int)fs0::value<float>(lhs));
	}

	bool eq( object_id lhs, object_id rhs ) {
		if ( o_type(lhs) == type_id::int_t ) {
			if ( o_type(lhs) == o_type(rhs) )
				return fs0::value<int>(lhs) == fs0::value<int>(rhs);
			return fs0::value<int>(lhs) == (int)fs0::value<float>(rhs);
		}
		if ( o_type(lhs) == o_type(rhs)) {
			return (int)fs0::value<float>(lhs) == (int)fs0::value<float>(rhs);
		}
		return fs0::value<int>(rhs) == (int)fs0::value<float>(lhs);
	}
}

bool SumFormula::_satisfied(const std::vector<object_id>& values) const {
	// sum(x_1, ..., x_n) meaning x_1 + ... + x_{n-1} = x_n
    if ( values.size() < 3 ) {
		throw std::runtime_error("sum(x_1,...,x_{n-1}) = x_n needs to have n > 2");
    }
	assert(values.size() > 1);
	object_id expected_sum = values.back();
	object_id addends_sum = values[0];
    for ( unsigned i = 1; i < values.size() - 1; i++ ) {
        addends_sum = sum_detail::add( addends_sum, values[i] );
    }

	return sum_detail::eq(addends_sum,expected_sum);
}

NValuesFormula::NValuesFormula(const NValuesFormula& formula) : NValuesFormula(Utils::clone(formula._subterms)) {}

bool NValuesFormula::_satisfied(const std::vector<object_id>& values) const {
	// nvalues(x_1, ..., x_n) meaning there are exactly x_n different values among variables <x_1, ... x_{n-1}>
	assert(values.size() > 1);
	assert(fs0::value<int>(values[values.size()-1]) > 0);
	std::set<object_id> distinct;
	for (unsigned i = 0; i < values.size() - 1; ++i) {
		distinct.insert(values[i]);
	}
	return distinct.size() == (std::size_t) fs0::value<int>(values[values.size()-1]);
}

} } } // namespaces
