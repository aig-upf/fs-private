#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/hybrid/base.hxx>

#ifdef FS_HYBRID
#include <fs/hybrid/heuristics/l2_norm.hxx>
#endif

#include <map>
#include <set>
#include <tuple>
#include <vector>

namespace fs0 {

//! Filters R-set so that points on the cartesian product lie within the "goal ball"
class GoalBallFilter {
public:

    FS_LP_METHOD(GoalBallFilter());

    ~GoalBallFilter() = default;

    void add_sample( unsigned feature_index, VariableIdx x, object_id value );
    void filter_samples();

    template <class Container, class FSFeatureValueT >
    void get_filtered_set( Container& new_R ) const {
        typedef typename Container::value_type value_type;
        for ( auto pair : _filtered_R_set ) {
            unsigned index;
            object_id v;
            std::tie( index, v) = pair;
            new_R.push_back( value_type(index, fs0::raw_value<FSFeatureValueT>(v)) );
        }

    }

    template <class Container, class FSFeatureValueT>
    void get_output( Container& out ) {
        typedef typename Container::value_type value_type;
        for ( auto p : _filtered_points ) {
            unsigned f0,f1;
            object_id v0,v1;
            std::tie( f0, v0, f1, v1) = p;
            out.push_back( value_type(f0, fs0::raw_value<FSFeatureValueT>(v0), f1, fs0::raw_value<FSFeatureValueT>(v1)));
        }
    }
protected:

    void compute_c0_values();

protected:

    std::map< unsigned, VariableIdx >                               _feat2var;
    std::map< VariableIdx, unsigned >                               _var2feat;
    std::set< VariableIdx >                                         _variables;
    std::map< VariableIdx, std::vector< object_id > >               _values;
    std::set<std::tuple<unsigned, object_id>>                       _filtered_R_set;
    std::set<std::tuple<unsigned, object_id, unsigned, object_id>>  _filtered_points;
    std::vector< std::tuple< VariableIdx, VariableIdx > >           _projections;
    std::map< std::tuple< VariableIdx, VariableIdx >, float >       _c0;
    unsigned                                                        _num_samples;

    FS_LP_ATTRIB(hybrid::L2Norm _norm)
};

} // Namespaces
