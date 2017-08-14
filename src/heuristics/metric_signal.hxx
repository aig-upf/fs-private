#pragma once

#include <languages/fstrips/language_fwd.hxx>
#include <heuristics/reward.hxx>
#include <memory>

namespace fs0 {
    class Problem;
}

namespace fs0 { namespace hybrid {
    class StateMetricSignal : public Reward {
    public:

        StateMetricSignal( const fs::Metric* metric);
        virtual ~StateMetricSignal() = default;

        virtual float evaluate( const State& s ) const override;

        static
        std::shared_ptr<Reward>
        create( const Problem& p );

    private:

        const fs::Metric*   _the_metric = nullptr;

    };

} }
