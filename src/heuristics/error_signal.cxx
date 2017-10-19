#include <heuristics/error_signal.hxx>
#include <cmath>
#include <lapkt/tools/logging.hxx>
#include <languages/fstrips/operations/basic.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

namespace hybrid {

    SquaredErrorSignal::~SquaredErrorSignal() {

    }

    std::shared_ptr<Reward>
    SquaredErrorSignal::create_from_goals( const Problem& p  ) {
        std::shared_ptr<SquaredErrorSignal> signal = std::make_shared<SquaredErrorSignal>();
        for ( auto formula : fs::all_formulae( *p.getGoalConditions())) {
            signal->addCondition(formula);
        }
        return signal;
    }

    void
    SquaredErrorSignal::addCondition( const fs::Formula* g) {
        const fs::RelationalFormula* formula = dynamic_cast<const fs::RelationalFormula*>( g );
        if ( formula == nullptr ) { // it is not
            _non_arithmetic_goal_condition.push_back(g);
            return;
        }

        if ( formula->symbol() == fs::RelationalFormula::Symbol::NEQ ) {
            _non_arithmetic_goal_condition.push_back(g);
            return;
        }

        _arithmetic_goal_condition.push_back(formula);
    }

    float
    SquaredErrorSignal::evaluate( const State& s) const {
        return -measure(s);
    }


    float
    SquaredErrorSignal::measure( const State& s ) const {

        auto norm = [](float v) { return 2.0f * (std::atan( v )/M_PI);};

        LPT_EDEBUG("error_signal", "State: " << s);
        double error = 0.0;
        // Atomic Conditions get handled as black boxes
        for ( auto g : _non_arithmetic_goal_condition ) {
            error = g->interpret( s ) ? error : error + 1.0;
            LPT_EDEBUG("error_signal", "\t sub-goal: " << *g );
            LPT_EDEBUG("error_signal", "\t\t error: " << error  );
        }

        // Numerical conditions get treated as follows
        for ( auto g : _arithmetic_goal_condition ) {
            if ( g->interpret(s) ) continue;


            float lhs_value = fs0::value<float>(g->lhs()->interpret(s));
            float rhs_value = fs0::value<float>(g->rhs()->interpret(s));

            float delta = std::fabs(lhs_value - rhs_value);
            // MRJ: the below is to avoid having the error become zero when
            // the condition is still not true
            if ( g->symbol() == fs::RelationalFormula::Symbol::GT
                || g->symbol() == fs::RelationalFormula::Symbol::LT )
                delta = std::max( (float)1e-6, delta);

            float e =  norm(delta);//(delta*delta);
            LPT_EDEBUG("error_signal", "\t sub-goal: " << *g );
            LPT_EDEBUG("error_signal", "\t\t delta: " << delta << " squared: " << e );
            //error += (2.0 * std::atan(std::log10(e+1e-2)) / M_PI);
            error += e;
        }

        //return error;
        //return (2.0 * std::atan(std::log10(error+1e-2)) / M_PI);
        LPT_EDEBUG("error_signal", "\t total error: " << error );
        //error =  norm(error);
        //LPT_INFO("error_signal", "\t normalised error: " << error );
        return error;
    }

}


}
