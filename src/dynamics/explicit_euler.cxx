#include <dynamics/explicit_euler.hxx>
#include <utils/config.hxx>

namespace fs0 { namespace dynamics { namespace integrators {

    ExplicitEuler::ExplicitEuler() {
        tmp = nullptr;
        integration_factor = Config::instance().getIntegrationFactor();
    }

    void
    ExplicitEuler::operator()( const State& s, const std::vector<DifferentialEquation>& f_expr,  State& next, double H, double factor ) const {

        const double base_duration = (H / factor);
        std::vector<Atom>   f_un; // f(u_n)

        while ( H > 0.0 ) {
            double h = std::min( base_duration, H  );

            evaluate_derivatives( next, f_expr, f_un );
            for ( unsigned i = 0; i < f_expr.size(); i++ ) {
                //! Euler method step
                //! u_{n+1} = u_{n} + h f(u_{n})
                float f_i = fs::value<float>(next.getValue( f_expr[i]._affected ));
                float f_un_i = fs::value<float>(f_un[i].getValue());
                float un1 = f_i + h * f_un_i;
                next.__set( f_expr[i]._affected, un1 );
            }
            H -= h;
        }

    }

}}}
