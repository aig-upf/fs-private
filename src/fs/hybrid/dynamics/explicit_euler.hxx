#pragma once

#include <fs/hybrid/dynamics/integrator.hxx>


namespace fs0 { namespace dynamics { namespace integrators {


    class ExplicitEuler : public Integrator
    {
        mutable State*  tmp;
    public:
        double      integration_factor;
        unsigned    order;

        //! Explicit Euler integration method
        //!
        //! y_{n+1} = y_{n} + h f(y_{n})
        //!
        //! where:
        //!
        //! y_n - current value of variable y
        //! y_{n+1} - next value of variable y, becomes the atom to be returned
        //! f() - differential equation to be evaluated, variable f_y_n
        //! h - the step size
        //!
        ExplicitEuler( );

        void operator()( const State& s, const std::vector<DifferentialEquation>& eq, State& next, double h, double factor ) const override;

    };


}}}
