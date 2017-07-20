#include <heuristics/l2_norm.hxx>
#include <cmath>

namespace fs0 { namespace hybrid {


    void
    L2Norm::add_condition( const fs::Formula* g) {
        _poly.add_constraint( g );
    }

    float
    L2Norm::measure( const State& s ) const {
        if ( _poly.solution().empty()) {
            _poly.setup();
            _poly.solve();
        }

        float accum = 0.0f;
        for ( const Atom& a : _poly.solution() ) {
            if ( o_type(a.getValue()) == type_id::int_t) {
                float delta = (float)fs0::value<int>(a.getValue()) - fs0::value<int>(s.getValue(a.getVariable()));
                accum += delta * delta;
            }
            else if ( o_type(a.getValue()) == type_id::float_t) {
                float delta = fs0::value<float>(a.getValue()) - fs0::value<float>(s.getValue(a.getVariable()));
                accum += delta * delta;
            }
        }
        return std::sqrt(accum);
    }
}}
