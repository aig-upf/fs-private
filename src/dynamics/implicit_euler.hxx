#pragma once

#include <dynamics/integrator.hxx>
#include <dynamics/explicit_euler.hxx>

namespace fs0 { namespace dynamics { namespace integrators {


    class ImplicitEuler : public Integrator
    {

        ExplicitEuler       _explicit_method;
        static unsigned     _max_iterations;
        mutable std::shared_ptr<State>  tmp;
    public:

        double              integration_factor;

        ImplicitEuler();

        //! Implicit Euler integration method via fixed point computation
        //!
        //
        // We're following a simple fixed-point computation here, with a twist,
        // y_{n+1}^{0} = explicit_euler(y_{n})
        // y_{n+1}^{i+1} = y_{n} + h f()
        //! where:
        //!
        //! y_n - current value of variable y
        //! y_{n+1} - next value of variable y, becomes the atom to be returned
        //! y'_{n+1} - next value of variable y, estimated using the explicit method
        //! f() - differential equation to be evaluated, stored in f_y
        //! h - the step size
        //!
        //! We proceed by:
        //! 1 - computing the estimation of y_{n+1} with the explicit method and
        //!     storing it into explicit_next_y
        //! 2 - compute the mid point
        //! 3 - evaluate f() on the state resulting from setting the affected variable
        //!     to the midpoint, this requires creating a temporary (TODO: optimise this out)
        //! 4 - Apply the update rule and return Atom with the new value

        void operator()( const State& s, const std::vector<DifferentialEquation>& eq, State& next, double H ) const override;
    };


}}}
