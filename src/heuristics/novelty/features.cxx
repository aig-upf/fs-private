
#include <problem_info.hxx>
#include <heuristics/novelty/features.hxx>
#include <state.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 {


FSFeatureValueT
StateVariableFeature::evaluate( const State& s ) const {
	return s.getIntValue(_variable);
}

std::ostream& StateVariableFeature::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return os << info.getVariableName(_variable);
}

FSFeatureValueT
ConditionSetFeature::evaluate( const State& s ) const {
	FSFeatureValueT satisfied = 0;
	for ( const auto c : _conditions ) {
		if (c->interpret(s)) satisfied++;
	}
	return satisfied;
}

std::ostream& ConditionSetFeature::print(std::ostream& os) const {
	return os << "UNIMPLEMENTED";
}

ArbitraryTermFeature::~ArbitraryTermFeature() {
	delete _term;
}

ArbitraryTermFeature::ArbitraryTermFeature(const ArbitraryTermFeature& other)
	: _term(other._term->clone())
{}

FSFeatureValueT
ArbitraryTermFeature::evaluate(const State& s) const {
	return boost::get<int>(_term->interpret(s));
}

std::ostream& ArbitraryTermFeature::print(std::ostream& os) const {
	return os << *_term;
}


ArbitraryFormulaFeature::~ArbitraryFormulaFeature() {
	delete _formula;
}

ArbitraryFormulaFeature::ArbitraryFormulaFeature(const ArbitraryFormulaFeature& other)
	: _formula(other._formula->clone())
{}

FSFeatureValueT
ArbitraryFormulaFeature::evaluate(const State& s) const {
	return _formula->interpret(s);
}

std::ostream& ArbitraryFormulaFeature::print(std::ostream& os) const {
	return os << *_formula;
}


}
