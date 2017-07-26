
#pragma once

#include <problem.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/language.hxx>
#include <state.hxx>

namespace fs0 {

namespace fs = fs0::language::fstrips;

namespace hybrid {

    class SquaredErrorSignal {
    public:

        SquaredErrorSignal() = default;

        ~SquaredErrorSignal();

        float measure( const State& s ) const;

        void addCondition( const fs::Formula* f );

    private:
        std::vector<const fs::Formula*>       _non_arithmetic_goal_condition;
        std::vector<const fs::RelationalFormula*>   _arithmetic_goal_condition;
    };

}

}
