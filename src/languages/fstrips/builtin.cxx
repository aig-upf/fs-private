
#include <numeric>

#include <languages/fstrips/builtin.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <state.hxx>

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


StaticHeadedNestedconst Term* ArithmeticTermFactory::create(const std::string& symbol, const std::vector<const Term*>& subterms, ProblemInfo::ObjectType value_type) {
	if (symbol == "+")      return new AdditionTerm(subterms, value_type);
	else if (symbol == "-") return new SubtractionTerm(subterms, value_type);
	else if (symbol == "*") return new MultiplicationTerm(subterms, value_type);
    else if (symbol == "/") return new DivisionTerm( subterms, value_type );
    else if (symbol == "^") return new PowerTerm( subterms, value_type);
    else if (symbol == "min") return new MinTerm( subterms, value_type);
    else if (symbol == "max") return new MaxTerm( subterms, value_type);
	else if (symbol == "sqrt") return new SqrtTerm( subterms, value_type);
	else if (symbol == "sin") return new SineTerm( subterms, value_type );
	else if (symbol == "cos") return new CosineTerm( subterms, value_type );
	else if (symbol == "tan") return new TangentTerm( subterms, value_type );
	else if (symbol == "asin") return new ArcSineTerm( subterms, value_type );
	else if (symbol == "acos") return new ArcCosineTerm( subterms, value_type );
	else if (symbol == "atan") return new ArcTangentTerm( subterms, value_type );
    else if (symbol == "exp") return new ArcTangentTerm( subterms, value_type );
	return nullptr;
}

AdditionTerm::AdditionTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {}

ObjectIdx AdditionTerm::interpret(const PartialAssignment& assignment) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(assignment)) + boost::get<int>(_subterms[1]->interpret(assignment));
	else
		res = boost::get<float>(_subterms[0]->interpret(assignment)) + boost::get<float>(_subterms[1]->interpret(assignment));
	return res;
}

ObjectIdx AdditionTerm::interpret(const State& state) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(state)) + boost::get<int>(_subterms[1]->interpret(state));
	else
		res = boost::get<float>(_subterms[0]->interpret(state)) + boost::get<float>(_subterms[1]->interpret(state));
	return res;
}

std::ostream& AdditionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " + " << *_subterms[1] << ")";
	return os;
}

SubtractionTerm::SubtractionTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {}

ObjectIdx SubtractionTerm::interpret(const PartialAssignment& assignment) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(assignment)) - boost::get<int>(_subterms[1]->interpret(assignment));
	else
		res = boost::get<float>(_subterms[0]->interpret(assignment)) - boost::get<float>(_subterms[1]->interpret(assignment));
	return res;
}

ObjectIdx SubtractionTerm::interpret(const State& state) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(state)) - boost::get<int>(_subterms[1]->interpret(state));
	else
		res = boost::get<float>(_subterms[0]->interpret(state)) - boost::get<float>(_subterms[1]->interpret(state));
	return res;
}


std::ostream& SubtractionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " - " << *_subterms[1] << ")";
	return os;
}


MultiplicationTerm::MultiplicationTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {}

ObjectIdx MultiplicationTerm::interpret(const PartialAssignment& assignment) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(assignment)) * boost::get<int>(_subterms[1]->interpret(assignment));
	else
		res = boost::get<float>(_subterms[0]->interpret(assignment)) * boost::get<float>(_subterms[1]->interpret(assignment));
	return res;
}

ObjectIdx MultiplicationTerm::interpret(const State& state) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(state)) * boost::get<int>(_subterms[1]->interpret(state));
	else
		res = boost::get<float>(_subterms[0]->interpret(state)) * boost::get<float>(_subterms[1]->interpret(state));
	return res;
}

std::ostream& MultiplicationTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " * " << *_subterms[1] << ")";
	return os;
}

DivisionTerm::DivisionTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {}

ObjectIdx DivisionTerm::interpret(const PartialAssignment& assignment) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(assignment)) / boost::get<int>(_subterms[1]->interpret(assignment));
	else
		res = boost::get<float>(_subterms[0]->interpret(assignment)) / boost::get<float>(_subterms[1]->interpret(assignment));
	return res;
}

ObjectIdx DivisionTerm::interpret(const State& state) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = boost::get<int>(_subterms[0]->interpret(state)) / boost::get<int>(_subterms[1]->interpret(state));
	else
		res = boost::get<float>(_subterms[0]->interpret(state)) / boost::get<float>(_subterms[1]->interpret(state));
	return res;
}


std::ostream& DivisionTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " / " << *_subterms[1] << ")";
	return os;
}

PowerTerm::PowerTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {}

ObjectIdx PowerTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = (int)std::pow(boost::get<int>(_subterms[0]->interpret(assignment)), boost::get<int>(_subterms[1]->interpret(assignment)));
	else
		res = (float)std::pow(boost::get<float>(_subterms[0]->interpret(assignment)), boost::get<float>(_subterms[1]->interpret(assignment)));
	return res;
}

ObjectIdx PowerTerm::interpret(const State& state) const {
	ObjectIdx res;
	if ( getValueType() == ObjectType::INT )
	 	res = (int)std::pow(boost::get<int>(_subterms[0]->interpret(state)), boost::get<int>(_subterms[1]->interpret(state)));
	else
		res = (float)std::pow(boost::get<float>(_subterms[0]->interpret(state)), boost::get<float>(_subterms[1]->interpret(state)));
	return res;
}

std::ostream& PowerTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "(" << *_subterms[0] << " ^ " << *_subterms[1] << ")";
	return os;
}

// Sqrt Term Implementation
SqrtTerm::SqrtTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// Square root is a unary arithmetic term
		throw std::runtime_error("Square root is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// Square root only defined for reals
		throw std::runtime_error("Square root is only defined for real numbers");
	}

}

ObjectIdx SqrtTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx res;
	res = (float)std::sqrt(boost::get<float>(_subterms[0]->interpret(assignment)));
	return res;
}

ObjectIdx SqrtTerm::interpret(const State& state) const {
	ObjectIdx res;
	res = (float)std::sqrt(boost::get<float>(_subterms[0]->interpret(state)));
	return res;
}

std::ostream& SqrtTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "sqrt(" << *_subterms[0] << ")";
	return os;
}

// Sine Term Implementation
SineTerm::SineTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// sin(x) a unary arithmetic term
		throw std::runtime_error("sin(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// sin(x) defined for reals
		throw std::runtime_error("sin(x) is only defined for real numbers");
	}

}

ObjectIdx SineTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx res;
	res = (float)std::sin(boost::get<float>(_subterms[0]->interpret(assignment)));
	return res;
}

ObjectIdx SineTerm::interpret(const State& state) const {
	ObjectIdx res;
	res = (float)std::sin(boost::get<float>(_subterms[0]->interpret(state)));
	return res;
}

std::ostream& SineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "sin(" << *_subterms[0] << ")";
	return os;
}

// Cosine Term Implementation
CosineTerm::CosineTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// cos(x) a unary arithmetic term
		throw std::runtime_error("cos(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// cps(x) defined for reals
		throw std::runtime_error("cos(x) is only defined for real numbers");
	}

}

ObjectIdx CosineTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx res;
	res = (float)std::cos(boost::get<float>(_subterms[0]->interpret(assignment)));
	return res;
}

ObjectIdx CosineTerm::interpret(const State& state) const {
	ObjectIdx res;
	res = (float)std::cos(boost::get<float>(_subterms[0]->interpret(state)));
	return res;
}

std::ostream& CosineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "cos(" << *_subterms[0] << ")";
	return os;
}

// Tanget Term Implementation
TangentTerm::TangentTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("tan(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// tan(x) defined for reals
		throw std::runtime_error("tan(x) is only defined for real numbers");
	}

}

ObjectIdx TangentTerm::interpret(const PartialAssignment& assignment) const {
	float v = boost::get<float>(_subterms[0]->interpret(assignment));
	// Check for singularity
	std::feclearexcept(FE_ALL_EXCEPT);
	float res = (float)std::tan(v);
	if(std::fetestexcept(FE_INVALID)) {
		throw std::runtime_error("FE_INVALID thrown while evaluating tan(x)!");
	}
	if ( std::isnan(res)) {
		throw std::runtime_error("Evaluating tan(x) resulted in a NaN result");
	}
	return ObjectIdx(res);
}

ObjectIdx TangentTerm::interpret(const State& state) const {
	float v = boost::get<float>(_subterms[0]->interpret(state));
	// Check for singularity
	std::feclearexcept(FE_ALL_EXCEPT);
	float res = (float)std::tan(v);
	if (std::fetestexcept(FE_INVALID)) {
		throw std::runtime_error("FE_INVALID thrown while evaluating tan(x)!");
	}
	if ( std::isnan(res)) {
		throw std::runtime_error("Evaluating tan(x) resulted in a NaN result");
	}
	return ObjectIdx(res);
}

std::ostream& TangentTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "tan(" << *_subterms[0] << ")";
	return os;
}

// Inverse Trigonometric Functions

// Sine Term Implementation
ArcSineTerm::ArcSineTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// asin(x) a unary arithmetic term
		throw std::runtime_error("asin(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// asin(x) defined for reals
		throw std::runtime_error("asin(x) is only defined for real numbers");
	}

}

ObjectIdx ArcSineTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx res;
	res = (float)std::asin(boost::get<float>(_subterms[0]->interpret(assignment)));
	return res;
}

ObjectIdx ArcSineTerm::interpret(const State& state) const {
	ObjectIdx res;
	res = (float)std::asin(boost::get<float>(_subterms[0]->interpret(state)));
	return res;
}

std::ostream& ArcSineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "asin(" << *_subterms[0] << ")";
	return os;
}


// Cosine Term Implementation
ArcCosineTerm::ArcCosineTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// acos(x) a unary arithmetic term
		throw std::runtime_error("acos(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// acos(x) defined for reals
		throw std::runtime_error("acos(x) is only defined for real numbers");
	}

}

ObjectIdx ArcCosineTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx res;
	res = (float)std::acos(boost::get<float>(_subterms[0]->interpret(assignment)));
	return res;
}

ObjectIdx ArcCosineTerm::interpret(const State& state) const {
	ObjectIdx res;
	res = (float)std::acos(boost::get<float>(_subterms[0]->interpret(state)));
	return res;
}

std::ostream& ArcCosineTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "acos(" << *_subterms[0] << ")";
	return os;
}

// Tanget Term Implementation
ArcTangentTerm::ArcTangentTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// atan(x) a unary arithmetic term
		throw std::runtime_error("atan(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// tan(x) defined for reals
		throw std::runtime_error("atan(x) is only defined for real numbers");
	}

}

ObjectIdx ArcTangentTerm::interpret(const PartialAssignment& assignment) const {
	float v = boost::get<float>(_subterms[0]->interpret(assignment));
	// Check for singularity
	std::feclearexcept(FE_ALL_EXCEPT);
	float res = (float)std::atan(v);
	if(std::fetestexcept(FE_INVALID)) {
		throw std::runtime_error("FE_INVALID thrown while evaluating atan(x)!");
	}
	if ( std::isnan(res)) {
		throw std::runtime_error("Evaluating atan(x) resulted in a NaN result");
	}
	return ObjectIdx(res);
}

ObjectIdx ArcTangentTerm::interpret(const State& state) const {
	float v = boost::get<float>(_subterms[0]->interpret(state));
	// Check for singularity
	std::feclearexcept(FE_ALL_EXCEPT);
	float res = (float)std::atan(v);
	if (std::fetestexcept(FE_INVALID)) {
		throw std::runtime_error("FE_INVALID thrown while evaluating tan(x)!");
	}
	if ( std::isnan(res)) {
		throw std::runtime_error("Evaluating atan(x) resulted in a NaN result");
	}
	return ObjectIdx(res);
}

std::ostream& ArcTangentTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "atan(" << *_subterms[0] << ")";
	return os;
}

// Tanget Term Implementation
ExpTerm::ExpTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() > 1 ) {
		// atan(x) a unary arithmetic term
		throw std::runtime_error("exp(x) is a unary function");
	}
	if ( value_type != ObjectType::NUMBER ) {
		// tan(x) defined for reals
		throw std::runtime_error("exp(x) is only defined for real numbers");
	}

}

ObjectIdx ExpTerm::interpret(const PartialAssignment& assignment) const {
	float v = boost::get<float>(_subterms[0]->interpret(assignment));
	// Check for singularity
	std::feclearexcept(FE_ALL_EXCEPT);
	float res = (float)std::exp(v);
	if(std::fetestexcept(FE_OVERFLOW)) {
		throw std::runtime_error("FE_OVERFLOW thrown while evaluating exp(x)!");
	}
	if ( std::isnan(res)) {
		throw std::runtime_error("Evaluating exp(x) resulted in a NaN result");
	}
	return ObjectIdx(res);
}

ObjectIdx ExpTerm::interpret(const State& state) const {
	float v = boost::get<float>(_subterms[0]->interpret(state));
	// Check for singularity
	std::feclearexcept(FE_ALL_EXCEPT);
	float res = (float)std::exp(v);
    if(std::fetestexcept(FE_OVERFLOW)) {
		throw std::runtime_error("FE_OVERFLOW thrown while evaluating exp(x)!");
	}
	if ( std::isnan(res)) {
		throw std::runtime_error("Evaluating exp(x) resulted in a NaN result");
	}
	return ObjectIdx(res);
}

std::ostream& ExpTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "exp(" << *_subterms[0] << ")";
	return os;
}

// Min Term Implementation
MinTerm::MinTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("min(x,y) is a binary function");
	}
}

ObjectIdx MinTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx lhs = _subterms[0]->interpret(assignment);
    ObjectIdx rhs = _subterms[1]->interpret(assignment);
    if (getValueType() == ObjectType::INT) {
        return ObjectIdx(std::min( boost::get<int>(lhs), boost::get<int>(rhs)));
    }
    return ObjectIdx(std::min( boost::get<float>(lhs), boost::get<float>(rhs)));

}

ObjectIdx MinTerm::interpret(const State& state) const {
    ObjectIdx lhs = _subterms[0]->interpret(assignment);
    ObjectIdx rhs = _subterms[1]->interpret(assignment);
    if (getValueType() == ObjectType::INT) {
        return ObjectIdx(std::min( boost::get<int>(lhs), boost::get<int>(rhs)));
    }
    return ObjectIdx(std::min( boost::get<float>(lhs), boost::get<float>(rhs)));
}

std::ostream& MinTerm::print(std::ostream& os, const fs0::ProblemInfo& info) const {
	os << "min(" << *_subterms[0] << ", " << *_subterms[1] << ")";
	return os;
}

// Max Term Implementation
MaxTerm::MaxTerm(const std::vector<const Term*>& subterms, ObjectType value_type)
	: ArithmeticTerm(subterms, value_type) {
	if ( subterms.size() != 2 ) {
		// tan(x) a unary arithmetic term
		throw std::runtime_error("max(x,y) is a binary function");
	}
}

ObjectIdx MaxTerm::interpret(const PartialAssignment& assignment) const {
    ObjectIdx lhs = _subterms[0]->interpret(assignment);
    ObjectIdx rhs = _subterms[1]->interpret(assignment);
    if (getValueType() == ObjectType::INT) {
        return ObjectIdx(std::max( boost::get<int>(lhs), boost::get<int>(rhs)));
    }
    return ObjectIdx(std::max( boost::get<float>(lhs), boost::get<float>(rhs)));

}

ObjectIdx MaxTerm::interpret(const State& state) const {
    ObjectIdx lhs = _subterms[0]->interpret(assignment);
    ObjectIdx rhs = _subterms[1]->interpret(assignment);
    if (getValueType() == ObjectType::INT) {
        return ObjectIdx(std::max( boost::get<int>(lhs), boost::get<int>(rhs)));
    }
    return ObjectIdx(std::max( boost::get<float>(lhs), boost::get<float>(rhs)));
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

bool SumFormula::_satisfied(const ObjectIdxVector& values) const {
	// sum(x_1, ..., x_n) meaning x_1 + ... + x_{n-1} = x_n
	assert(values.size() > 1);
	int expected_sum = values.back();
	int addends_sum  = std::accumulate(values.begin(), values.end() - 1, 0);
	return addends_sum == expected_sum;
}

NValuesFormula::NValuesFormula(const NValuesFormula& formula) : NValuesFormula(Utils::clone(formula._subterms)) {}

bool NValuesFormula::_satisfied(const ObjectIdxVector& values) const {
	// nvalues(x_1, ..., x_n) meaning there are exactly x_n different values among variables <x_1, ... x_{n-1}>
	assert(values.size() > 1);
	assert(values[values.size()-1] > 0);
	std::set<ObjectIdx> distinct;
	for (unsigned i = 0; i < values.size() - 1; ++i) {
		distinct.insert(values[i]);
	}
	return distinct.size() == (std::size_t) values[values.size()-1];
}

} } } // namespaces
