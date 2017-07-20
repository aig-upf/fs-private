#pragma once

#include <problem.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/language.hxx>
#include <state.hxx>
#include <constraints/soplex/lp.hxx>

namespace fs0 {

namespace fs = fs0::language::fstrips;

namespace hybrid {

    class L2Norm {
    public:
        //typedef  std::function<float(object_id,object_id)>  DiffOp;

        L2Norm() = default;

        ~L2Norm() = default;

        float measure( const State& s ) const;

        void add_condition( const fs::Formula* f );

    private:
        mutable spx::LinearProgram               _poly;
    };

}

}
