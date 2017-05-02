#pragma once

#include <dynamics/integrator.hxx>


namespace fs0 { namespace dynamics { namespace integrators {


    class RungeKutta2 : public Integrator
    {
        mutable State*  tmp;
    public:
        double      integration_factor;
        unsigned    order;

        //! Runge Kutta 2nd order integration method
        //!
        //! y_{n+(1/2)} = y_{n} + h/2 f(y_{n})
        //! y_{n+1} = y_{n} + h f(y_{n+(1/2)})
        //!
        //! where:
        //!
        //! y_n - current value of variable y
        //! y_{n+1} - next value of variable y, becomes the atom to be returned
        //! f() - differential equation to be evaluated, variable f_y_n
        //! h - the step size
        //!
        RungeKutta2( );

        void operator()( const State& s, const std::vector<DifferentialEquation>& eq, State& next, double h ) const override;


    };


}}}
