#include <heuristics/metric_signal.hxx>
#include <languages/fstrips/metrics.hxx>
#include <problem.hxx>

namespace fs0{ namespace hybrid {

    StateMetricSignal::StateMetricSignal( const fs::Metric* metric )
        : _the_metric( metric ) {}

    float
    StateMetricSignal::evaluate( const State& s ) const {
        object_id v = _the_metric->apply(s);
        float unwrapped_v = fs0::value<float>(v);
        return _the_metric->optimization() == MetricType::MINIMIZE ? -unwrapped_v  : unwrapped_v ;
    }

    std::shared_ptr<Reward>
    StateMetricSignal::create( const Problem& p ) {
        if ( p.get_metric() == nullptr )
            throw std::runtime_error("StateMetric::create() : the problem has no metric defined!");
        return std::make_shared<StateMetricSignal>( p.get_metric() );
    }

}}
