
#include <problem_info.hxx>
#include <heuristics/novelty/features.hxx>
#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <algorithm>

namespace fs0 {

namespace fs = fs0::language::fstrips;

Feature::Feature()
	: _codomain( type_id::invalid_t ) {
}

Feature::Feature( const std::vector<VariableIdx>& S, type_id T )
	: _scope(S), _codomain(T) {}

Feature::~Feature() {

}

StateVariableFeature::StateVariableFeature( VariableIdx x )
	: Feature( {x}, ProblemInfo::getInstance().sv_type(x) ),
	_variable(x) {
}

FSFeatureValueT
StateVariableFeature::evaluate( const State& s ) const {
	object_id v = s.getValue(_variable);
	// MRJ: Added saturation rule for floating point numbers,
	// so they all become 0 if they're very small
	if (o_type(v) == type_id::float_t )  {
		if ( std::fabs(fs0::value<float>(v)) < 1e-6 )
			v = make_object<float>(0.0f);
	}
	return fs0::raw_value<FSFeatureValueT>(v);
}

std::ostream& StateVariableFeature::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return os << info.getVariableName(_variable);
}

ConditionSetFeature::ConditionSetFeature()
	: Feature({}, type_id::int_t) {

}

ConditionSetFeature::~ConditionSetFeature() {
	for ( auto f : _conditions )
		delete f;
}

void
ConditionSetFeature::addCondition(const fs::Formula* condition) {
	// update the scope
	std::set<VariableIdx> condS;
	fs::ScopeUtils::computeFullScope(condition, condS);
	// MRJ: Note that feature now owns the formulae it is wrapping up
	_conditions.push_back(condition->clone());
	for ( auto x : condS ) {
		if ( std::find(_scope.begin(),_scope.end(), x) == _scope.end()) continue;
		_scope.push_back(x);
	}
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
	return  fs0::value<FSFeatureValueT>(_term->interpret(s));
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
