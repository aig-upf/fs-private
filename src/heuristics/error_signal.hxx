
#pragma once

#include <problem.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/language.hxx>
#include <state.hxx>
#include <heuristics/reward.hxx>
#include <memory>

namespace fs0 {

namespace fs = fs0::language::fstrips;

namespace hybrid {

    //! Squared Error Signal
    //!
    //! Follows from J. Lohr's discussion of heuristic guidance
    //! in Chapter 3 of his PhD thesis
    class SquaredErrorSignal : public Reward {
    public:

        SquaredErrorSignal() = default;

        ~SquaredErrorSignal();

        float measure( const State& s ) const;

        virtual float evaluate( const State& s ) const override;

        void addCondition( const fs::Formula* f );

        static
        std::shared_ptr<Reward>
        create_from_goals(const Problem& p);

    private:
        std::vector<const fs::Formula*>             _non_arithmetic_goal_condition;
        std::vector<const fs::RelationalFormula*>   _arithmetic_goal_condition;
    };

}

}
