#pragma once

#include <fs/core/heuristics/novelty/features.hxx>
#include <fs/core/heuristics/error_signal.hxx>

namespace fs0 {

    /**
        Squared Error Feature
        =====================

        This feature maps states into real numbers representing the distance
        L^2 norm (Euclidean distance) between s and the region of the state
        space implicitly defined by the set of conditions considered.

        When all the conditions are of the form

        x_0 = v_0 & x_1 = v_1 & ... & x_n = v_n

        the feature is exactly the distance between two points in R^n.

        When conditions are of the form

        \bigwedge expr_{i}^{l} <op> expr_{i}^{r}

        each condition is evaluated separatedly, and if found to be not
        true in s, then the distance between the point given by s and
        the plane given by each condition is calculated directly as

        || [expr_{i}^l]^s - [expr_{i}^r]^s ||

    */
    class SquaredErrorFeature : public ConditionSetFeature {
    public:
    	SquaredErrorFeature();

    	void 								addCondition(const fs::Formula* condition) override;
    	FSFeatureValueT 					evaluate(const State& s) const override;
    	const hybrid::SquaredErrorSignal&	error_signal() const { return _error; }
    protected:
    	hybrid::SquaredErrorSignal	_error;
    };

}
