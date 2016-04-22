
#include <heuristics/novelty/features.hxx>
#include <state.hxx>

namespace fs0 {

aptk::ValueIndex StateVariableFeature::evaluate( const State& s ) const { return s.getValue(_variable); }

aptk::ValueIndex ConditionSetFeature::evaluate( const State& s ) const {
	aptk::ValueIndex satisfied = 0;
	for ( const fs::AtomicFormula* c : _conditions ) {
		if ( c->interpret( s ) ) satisfied++;
	}
	return satisfied;
}

}
