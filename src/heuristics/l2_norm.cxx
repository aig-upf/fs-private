#include <heuristics/l2_norm.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/operations/basic.hxx>

#include <lapkt/tools/logging.hxx>

#include <cmath>
#include <algorithm>


namespace fs = fs0::language::fstrips;


namespace fs0 { namespace hybrid {

    L2Norm::L2Norm( const Problem& p ) {
        for ( auto formula : fs::all_relations( *p.getGoalConditions() ) ) {
            add_condition(formula);
        }
        calibrate(p.getInitialState());
    }

    void
    L2Norm::add_condition( const fs::Formula* g) {
        _poly.add_constraint( g );
    }

    void
    L2Norm::calibrate( const State& s ) {
        _cmin = measure(s);
        unsigned num_levels = 10;
        float delta_c = _cmin / (float) num_levels;
        float c_k = _cmin;
        while (c_k > delta_c) {
            _bands.push_back(c_k);
            c_k -= delta_c;
        }
    }

    unsigned
    L2Norm::ball_geodesic_index( const State& s ) const {
        float c_s = measure(s);
        LPT_DEBUG("heuristic", "Geodesic index:");
        LPT_DEBUG("heuristic", "state: " << s);
        LPT_DEBUG("heuristic", "h(s) = " << c_s );

        unsigned index = 0;
        unsigned k;
        for (  k = 0; k < _bands.size(); k++ )
            if ( c_s > _bands[k] ) {
                index = _bands.size() - k;
                break;
            }
        LPT_DEBUG("heuristic", "bin index: " << k << " h: " << _bands[k] << " index: " << index);
        return index;
    }

    float
    L2Norm::measure( const std::vector<VariableIdx>& S, const State& a, const State& b ) {
        float accum = 0.0f;
        for ( VariableIdx x : S ) {
            if ( o_type(a.getValue(x)) == type_id::int_t) {
                float delta = (float)fs0::value<int>(a.getValue(x)) - fs0::value<int>(b.getValue(x));
                accum += delta * delta;
            }
            else if ( o_type(a.getValue(x)) == type_id::float_t) {
                float delta = fs0::value<float>(a.getValue(x)) - fs0::value<float>(b.getValue(x));
                accum += delta * delta;
            }
        }
        return std::sqrt(accum);
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

    float
    L2Norm::measure( const std::vector<VariableIdx>& scope, const State& s ) const {
        if ( _poly.solution().empty()) {
            _poly.setup();
            _poly.solve();
        }

        float accum = 0.0f;
        std::vector<Atom> relevant;
        for ( const Atom& a : _poly.solution() )
            if ( std::find( scope.begin(), scope.end(), a.getVariable() ) != scope.end() )
                relevant.push_back(a);

        for ( const Atom& a : relevant ) {
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
