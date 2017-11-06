
#include <modules/hybrid/heuristics/triangle_inequality.hxx>
#include <problem.hxx>

namespace fs0 {

    TriangleInequality::TriangleInequality()
    	: ConditionSetFeature() {
    	_codomain = type_id::float_t;
        _init = std::make_shared<State>( Problem::getInstance().getInitialState());
    }

    void
    TriangleInequality::set_init( const State& init ) {
        _init = std::make_shared<State>( init );
    }

    void
    TriangleInequality::addCondition(const fs::Formula* condition) {
    	ConditionSetFeature::addCondition(condition);
    	_norm.add_condition(condition);
    }

    FSFeatureValueT
    TriangleInequality::evaluate( const State& s ) const {
    	return fs0::raw_value<FSFeatureValueT>( make_object( hybrid::L2Norm::measure(_scope, *_init, s) + _norm.measure(s)) );
    }

}
