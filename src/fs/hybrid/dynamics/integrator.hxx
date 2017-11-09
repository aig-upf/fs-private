#pragma once

#include <memory>
#include <fs/hybrid/dynamics/ode.hxx>
#include <fs/core/state.hxx>

namespace fs0 { namespace dynamics { namespace integrators {

    class Integrator {

    public:

        typedef std::shared_ptr<Integrator> sptr;
        void evaluate_derivatives( const State& s, const std::vector<DifferentialEquation>& f_expr, std::vector<Atom>& update ) const;

        Integrator();
        virtual ~Integrator();
        virtual void operator()( const State& s, const std::vector<DifferentialEquation>& f_expr, State& next,  double H, double factor) const = 0;


        mutable unsigned _num_evals;
    };

}}}
