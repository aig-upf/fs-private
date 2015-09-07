
#include <heuristics/novelty/features.hxx>
#include <state.hxx>

namespace fs0 {

aptk::ValueIndex StateVarFeature::evaluate( const State& s ) const { return s.getValue(_variable); }


aptk::ValueIndex ConstraintSetFeature::evaluate( const State& s ) const {
	aptk::ValueIndex value = 0;
	for ( fs::AtomicFormula::cptr c : _conditions ) {
		if ( c->interpret( s ) ) value++;
	}
	return value;
}

}
