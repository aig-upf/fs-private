#pragma once

#include <fs/core/heuristics/novelty/features.hxx>
#include <fs/hybrid/heuristics/l2_norm.hxx>
#include <memory>

namespace fs0 {

    /**
        Triangle Inequality Feature
        ===========================

        This feature maps states s into a lower bound of the shortest distance
        traversed by a trajectory of states starting with s_{init} and ending in
        s_{C} constrained to get through s, i.e.

            phi(s) = ||s_{init} - s||_{2} + ||s - s_{C}||_2

        init is a given state and s_{C} is defined in terms of a set of conditions.
    */
    class TriangleInequality : public ConditionSetFeature {
    public:
    	TriangleInequality();

        void                                set_init( const State& s );
    	void 								addCondition(const fs::Formula* condition) override;
    	FSFeatureValueT 					evaluate(const State& s) const override;
    	const hybrid::L2Norm&	            norm() const { return _norm; }
    protected:
    	hybrid::L2Norm	                _norm;
        std::shared_ptr<State>          _init;
    };

}
