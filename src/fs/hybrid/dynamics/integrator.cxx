#include <fs/hybrid/dynamics/integrator.hxx>

namespace fs0 { namespace dynamics { namespace integrators {

    Integrator::Integrator()
        : _num_evals(0) {

    }

    Integrator::~Integrator() {

    }

    void
    Integrator::evaluate_derivatives( const State& s, const std::vector<DifferentialEquation>& f_expr, std::vector<Atom>& update) const {
        assert(f_expr.size() == update.size());
        for ( unsigned i = 0; i < f_expr.size(); i++ ) {
            float delta_xi = 0.0f;
            for ( unsigned j = 0; j < f_expr[i]._terms.size(); j++ ) {
                auto expr = f_expr[i]._terms[j];
                double sign = f_expr[i]._signs[j];
                float interpretation = fs0::value<float>(expr->interpret( s ));
                delta_xi += (sign * (double)interpretation);
            }
            update[i] = Atom( f_expr[i]._affected, make_object<float>(delta_xi) );
        }
        _num_evals++;
    }

}}}
