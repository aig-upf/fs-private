#include <fs/hybrid/dynamics/runge_kutta_4.hxx>

#include <fs/core/utils/config.hxx>

namespace fs0 { namespace dynamics { namespace integrators {

    RungeKutta4::RungeKutta4() {
        tmp = nullptr;
    }

    void
    RungeKutta4::operator()( const State& s, const std::vector<DifferentialEquation>& f_expr, State& next, double H, double factor) const {

        const double base_duration = (H / factor);
        std::vector<Atom> k1( f_expr.size(), Atom( INVALID_VARIABLE, object_id::INVALID ) );
        std::vector<Atom> k2( f_expr.size(), Atom( INVALID_VARIABLE, object_id::INVALID ) );
        std::vector<Atom> k3( f_expr.size(), Atom( INVALID_VARIABLE, object_id::INVALID ) );
        std::vector<Atom> k4( f_expr.size(), Atom( INVALID_VARIABLE, object_id::INVALID ) );
        std::vector<Atom> un( f_expr.size(), Atom( INVALID_VARIABLE, object_id::INVALID ) );

        while ( H > 0.0 ) {
            double h = std::min( base_duration, H  );

            // Evaluate increment based on the slope at the beginning of the interval
            evaluate_derivatives( next, f_expr, k1 );
            for ( unsigned i = 0; i < f_expr.size(); i++ ) {
                //! Save values at step n
                un[i] = Atom( f_expr[i]._affected, next.getValue(f_expr[i]._affected));
                //! Half a step
                float f_i = fs0::value<float>(next.getValue( f_expr[i]._affected ));
                float k1_i = fs0::value<float>(k1[i].getValue());
                float un1 =  f_i + 0.5f * h * k1_i;
                next.__set( f_expr[i]._affected, un1 );
            }

            // Evaluate increment based on the slope at the midpoint of the interval
            evaluate_derivatives( next, f_expr, k2 );
            for ( unsigned i = 0; i < f_expr.size(); i++ ) {
                float un_i = fs0::value<float>(un[i].getValue());
                float k2_i = fs0::value<float>(k2[i].getValue());
                float un1 = un_i +  0.5f * h * k2_i ;
                next.__set( f_expr[i]._affected, un1 );
            }

            //! Again at the midpoint, but using the previous value
            evaluate_derivatives( next, f_expr, k3 );
            for ( unsigned i = 0; i < f_expr.size(); i++ ) {
                float un_i = fs0::value<float>(un[i].getValue());
                float k3_i = fs0::value<float>(k3[i].getValue());
                float un1 = un_i +  h * k3_i;
                next.__set( f_expr[i]._affected, un1 );
            }

            //! Increment at the end of the interval
            evaluate_derivatives( next, f_expr, k4 );
            //! Combine the values at each of the four points
            for ( unsigned i = 0; i < f_expr.size(); i++ ) {
                float k1_i = fs0::value<float>(k1[i].getValue());
                float k2_i = fs0::value<float>(k2[i].getValue());
                float k3_i = fs0::value<float>(k3[i].getValue());
                float k4_i = fs0::value<float>(k4[i].getValue());
                float blend = h/6.0f * (k1_i + 2.0f * k2_i + 2.0f * k3_i + k4_i);
                float un_i = fs0::value<float>(un[i].getValue());
                next.__set( f_expr[i]._affected, un_i + blend);
            }

            H -= h;
        }

    }

}}}
