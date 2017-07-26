#pragma once

#include <problem.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/language.hxx>
#include <state.hxx>
#include <constraints/soplex/lp.hxx>
#include <vector>

namespace fs0 {

namespace fs = fs0::language::fstrips;

namespace hybrid {

    class L2Norm {
    public:
        //typedef  std::function<float(object_id,object_id)>  DiffOp;

        L2Norm() = default;
        L2Norm( const Problem& prob );

        ~L2Norm() = default;

        // MRJ: sets up the reference point
        void        calibrate( const State& s );

        // MRJ: returns the index of the "band" in the geodesic
        unsigned    ball_geodesic_index( const State& s ) const;

        float measure( const State& s ) const;
        float measure( const std::vector<VariableIdx>& S, const State& a ) const;

        static float measure( const std::vector<VariableIdx>& S, const State& a, const State& b );

        void add_condition( const fs::Formula* f );

    private:
        mutable spx::LinearProgram               _poly;
        float                                    _cmin;
        std::vector<float>                       _bands;
    };

}

}
