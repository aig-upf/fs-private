#include <fs/core/heuristics/metric_signal.hxx>
#include <fs/core/languages/fstrips/metrics.hxx>
#include <fs/core/problem.hxx>

namespace fs0{ namespace hybrid {

    StateMetricSignal::StateMetricSignal( const fs::Metric* metric )
        : _the_metric( metric ) {}

    float
    StateMetricSignal::evaluate( const State& s ) const {
        float v = _the_metric->stage_cost(s);
        return _the_metric->optimization() == MetricType::MINIMIZE ? -v  : v ;
    }

    float
    StateMetricSignal::terminal( const State& s ) const {
        if ( _the_metric->terminal_expr() == nullptr ) return 0.0f;
        float v = _the_metric->terminal_cost(s);
        return _the_metric->optimization() == MetricType::MINIMIZE ? -v  : v ;
    }

    std::shared_ptr<Reward>
    StateMetricSignal::create( const Problem& p ) {
        if ( p.get_metric() == nullptr )
            throw std::runtime_error("StateMetric::create() : the problem has no metric defined!");
        return std::make_shared<StateMetricSignal>( p.get_metric() );
    }

}}
