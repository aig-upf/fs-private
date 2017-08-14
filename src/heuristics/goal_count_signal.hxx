#pragma once

#include <heuristics/reward.hxx>
#include <heuristics/unsat_goal_atoms.hxx>


namespace fs0 {
    class Problem;
}

namespace fs0 { namespace hybrid {

class GoalCountSignal : public Reward {
public:

    GoalCountSignal( const Problem& p);
    virtual ~GoalCountSignal() = default;

    virtual float evaluate( const State& s ) const override;

    static
    std::shared_ptr<Reward>
    create( const Problem& p );

private:

    UnsatisfiedGoalAtomsHeuristic   _h;

};

} }
