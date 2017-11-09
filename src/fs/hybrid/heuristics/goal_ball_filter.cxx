
#include <fs/hybrid/heuristics/goal_ball_filter.hxx>

#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations/basic.hxx>

#include <lapkt/tools/logging.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

    GoalBallFilter::GoalBallFilter()
    : _num_samples(0) {
        // MRJ: By default the goal is the "actual" goal
        for ( auto formula : fs::all_relations( *Problem::getInstance().getGoalConditions())) {
            _norm.add_condition(formula);
        }
    }

    void
    GoalBallFilter::add_sample( unsigned index, VariableIdx x, object_id v) {
        auto res = _variables.insert(x);
        if ( res.second ) {
            _feat2var.insert(std::make_pair(index,x));
            _var2feat.insert(std::make_pair(x,index));
        }
        auto it = _values.find(x);
        if (  it == _values.end() ) {
            // create new entry
            std::vector<object_id> x_values = { v };
            auto res = _values.insert( std::make_pair( x, x_values ));
            it = res.first;
        } else {
            it->second.push_back( v );
        }
        _num_samples++;
    }

    void
    GoalBallFilter::filter_samples() {
        // MRJ: setup projections
        std::vector<VariableIdx> tmp( _variables.begin(), _variables.end());
        for ( unsigned i = 0; i < tmp.size(); i++ ) {
            for ( unsigned j  = i+1; j < tmp.size(); j ++ ) {
                _projections.push_back( std::make_tuple(tmp[i],tmp[j]) );
            }
        }

        compute_c0_values();

        State s_x(Problem::getInstance().getInitialState());
        VariableIdx x0, x1;
        std::vector<VariableIdx> scope;
        std::vector<Atom>        point;
        for ( auto subspace : _projections ) {
            std::tie(x0,x1) = subspace;
            scope = {x0,x1};

            for ( object_id o_x0 : _values[x0] ) {
                for ( object_id o_x1 : _values[x1] ) {
                    point = {Atom(x0, o_x0), Atom(x1, o_x1)};
                    s_x.accumulate(point); // MRJ: Inefficient - accumulate calls updateHash()
                    float cp_x0_x1 = _norm.measure( scope, s_x );
                    if ( cp_x0_x1 > _c0[subspace] ) continue;
                    _filtered_R_set.insert( std::make_tuple( _var2feat[x0], o_x0 ) );
                    _filtered_R_set.insert( std::make_tuple( _var2feat[x1], o_x1 ) );
                    _filtered_points.insert( std::make_tuple( _var2feat[x0], o_x0, _var2feat[x1], o_x1 ) );
                }
            }
        }

        LPT_INFO("heuristic", "GoalBallFilter:::filter_samples() : generated filtered R set with " << _filtered_R_set.size() << " entries out of " << _num_samples );
        LPT_INFO("heuristic", "GoalBallFilter:::filter_samples() : filtered_points " << _filtered_points.size() );
    }

    void
    GoalBallFilter::compute_c0_values() {
        VariableIdx x0, x1;
        for ( auto subspace : _projections ) {
            std::tie(x0,x1) = subspace;
            std::vector<VariableIdx> scope = {x0,x1};
            float c0_x0_x1 = _norm.measure( scope, Problem::getInstance().getInitialState());
            _c0[subspace] = c0_x0_x1;
        }
    }

}
