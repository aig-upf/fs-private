#pragma once

#include <dynamics/integrator.hxx>


namespace fs0 { namespace dynamics { namespace integrators {


    class RungeKutta4 : public Integrator
    {
        mutable State*  tmp;
    public:
        double      integration_factor;
        unsigned    order;

        //! Runge Kutta 2nd order integration method
        //!
        //! k_1 = f(y_{n}) h
        //! k_2 = f(y_{n} + h/2 k_1)
        //! k_3 = f(y_{n} + h/2 k_2)
        //! k_4 = f(y_{n} + h k_3 )
        //! y_{n+1} = y_{n} + h/6 (k_1 + 2 k_2 + 2 k_3 + k_4 )
        //! where:
        //!
        //! y_n - current value of variable y
        //! y_{n+1} - next value of variable y, becomes the atom to be returned
        //! f() - differential equation to be evaluated, variable f_y_n
        //! h - the step size
        //!
        RungeKutta4( );

        void operator()( const State& s, const std::vector<DifferentialEquation>& eq, State& next, double h ) const override;


    };


}}}
