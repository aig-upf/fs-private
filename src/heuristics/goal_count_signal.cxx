#include <heuristics/goal_count_signal.hxx>
#include <languages/fstrips/formulae.hxx>
#include <problem.hxx>
#include <state.hxx>

namespace  fs0 { namespace hybrid {

    GoalCountSignal::GoalCountSignal( const Problem& p )
        : _h( p ) {

    }

    float
    GoalCountSignal::evaluate( const State& s ) const {
        return -_h.evaluate(s);
    }

    std::shared_ptr<Reward>
    GoalCountSignal::create( const Problem& p ) {
        return std::make_shared<GoalCountSignal>(p);
    }

}}
