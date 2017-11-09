#include <fs/core/heuristics/novelty/squared_error.hxx>

namespace fs0 {

    SquaredErrorFeature::SquaredErrorFeature()
    	: ConditionSetFeature() {
    	_codomain = type_id::float_t;
    }

    void
    SquaredErrorFeature::addCondition(const fs::Formula* condition) {
    	ConditionSetFeature::addCondition(condition);
    	_error.addCondition(condition);
    }

    FSFeatureValueT
    SquaredErrorFeature::evaluate( const State& s ) const {
    	return fs0::raw_value<FSFeatureValueT>( make_object(_error.measure(s)) );
    }

}
