#include "external.hxx"

#include <fs/core/constraints/registry.hxx>
#include <tuple>
#include <lapkt/tools/logging.hxx>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

using namespace rapidjson;

namespace boost_fs = boost::filesystem;

External::External( const ProblemInfo& info, const std::string& data_dir )
    : _vehicle(info.getTypeObjects("vehicle")), _data_dir(data_dir)
{
    //! map layout file
    unsigned track_layout_sym_id = info.getSymbolId( "track_layout" );
    _track_layout = info.resolveStateVariable( track_layout_sym_id, {});
    unsigned x_sym_id = info.getSymbolId("x");
    unsigned y_sym_id = info.getSymbolId("y");
    for (object_id o : _vehicle ) {
        _x[o] = info.resolveStateVariable( x_sym_id, {o});
        _y[o] = info.resolveStateVariable( y_sym_id, {o});
    }
    LPT_INFO("agv_external", "Bound track_layout(), x() and y() variables");
}

External::~External() {

}

std::shared_ptr<TrackLayout>
External::resolve_layout( object_id the_layout ) const {

    auto it = _layouts.find(the_layout);

    if ( it != _layouts.end() )
        return it->second;

    std::string map_name = ProblemInfo::getInstance().object_name( the_layout ) + ".json";

    boost_fs::path track_file = boost_fs::path(_data_dir) / boost_fs::path("layouts") / boost_fs::path(map_name);

    if (!boost_fs::exists(track_file))
        throw std::runtime_error("[AGV::External]: Could not open track data file '" + track_file.string() + "' for map '" + map_name + "'");
    if (!boost_fs::is_regular_file(track_file))
        throw std::runtime_error("[AGV::External]: Track data file '" + track_file.string() + "' cannot be opened");

    std::ifstream input(track_file.string());
    IStreamWrapper isw(input);
    Document d;
    d.ParseStream(isw);
    LPT_INFO("agv_external", "Loaded tracks from JSON document" << track_file );

    auto new_layout = std::make_shared<TrackLayout>();

    const Value& tracks = d["tracks"];
    assert( tracks.IsArray() );

    for ( SizeType i = 0; i < tracks.Size(); i++ ) {
        const Value& track_name = tracks[i]["name"];
        new_layout->_tracks[ std::string( track_name.GetString(), track_name.GetStringLength() ) ] = i;
        const Value& v = tracks[i]["v"];
        assert( v.IsArray() );
        fvec v_vec = { (float)v[0].GetDouble(), (float)v[1].GetDouble()};
        const Value& w = tracks[i]["w"];
        assert( w.IsArray() );
        fvec w_vec = { (float)w[0].GetDouble(), (float)w[1].GetDouble()};
        const Value& l = tracks[i]["left"];
        assert( l.IsArray() );
        fvec l_vec = { (float)l[0].GetDouble(), (float)l[1].GetDouble()};
        new_layout->_track_starts.push_back(v_vec);
        new_layout->_track_ends.push_back(w_vec);
        new_layout->_track_left_point.push_back(l_vec);
    }
    LPT_INFO("agv_external", "Loaded " << tracks.Size() << " track segments");
    _layouts.insert(std::make_pair(the_layout, new_layout));
    return new_layout;
}

void
External::registerComponents() const {
    LPT_INFO("agv_external", "Registering Components...");

    LogicalComponentRegistry::instance().
        addTermCreator( "@e", [](const std::vector<const fs::Term*>& subterms){ return new TrackDistance(subterms); });
    LPT_INFO("agv_external", "Registered @e...");
}


TrackDistance::TrackDistance(const std::vector<const fs::Term*>& subterms)
    : AxiomaticTerm( ProblemInfo::getInstance().getSymbolId( "@e" ), subterms ),
    _external( dynamic_cast<const External&>(ProblemInfo::getInstance().get_external())) {
}

TrackDistance*
TrackDistance::clone( const std::vector< const fs::Term*>& subterms ) const {
    return new TrackDistance(subterms);
}

namespace detail {

    float length_squared( fvec v, fvec w ) {
        fvec l = w -  v;
        return l[0] * l[0] + l[1] * l[1];
    }

    float distance( fvec p, fvec v ) {
        return std::sqrt( length_squared( p, v) );
    }

    float dot( fvec v, fvec w ) {
        return v[0]*w[0] + v[1]*w[1];
    }

    float side( fvec p, fvec v, fvec w) {
        //https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located
        return  (p[0]-v[0])*(w[1]-v[1]) - (p[1]-v[1])*(w[0]-v[0]);
    }

    float minimum_distance(fvec p, fvec v, fvec w) {
        // MRJ: https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
        // Return minimum distance between line segment vw and point p
        const float l2 = length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
        if (l2 == 0.0) return distance(p, v);   // v == w case
        // Consider the line extending the segment, parameterized as v + t (w - v).
        // We find projection of point p onto the line.
        // It falls where t = [(p-v) . (w-v)] / |w-v|^2
        // We clamp t from [0,1] to handle points outside the segment vw.
        const float t = std::max(0.0f, std::min(1.0f, dot(p - v, w - v) / l2));
        const fvec projection = v + t * (w - v);  // Projection falls on the segment
        return distance(p, projection);
    }

    bool left_to_right( fvec v, fvec w) {
        return w[0] - v[0] >= 0.0f;
    }

    bool upwards( fvec v, fvec w ) {
        return w[1] - v[1] >= 0.0f;
    }
}

object_id
TrackDistance::compute( const State& state, std::vector<object_id>& arguments) const {
    object_id vehicle = arguments[0];
    object_id x = state.getValue(_external._x.at(vehicle));
    object_id y = state.getValue(_external._y.at(vehicle));
    object_id the_layout = state.getValue(_external._track_layout);

    auto layout = _external.resolve_layout(the_layout);

    fvec p_v = {    fs0::value<float>(x),
                    fs0::value<float>(y)};

    float min_dist = 1e20;
    float sign = 1.0;

    for ( unsigned i = 0; i < layout->_track_starts.size(); i++ ) {
        float distance = detail::minimum_distance( p_v, layout->_track_starts[i], layout->_track_ends[i]);
        LPT_DEBUG("agv_external", "Distance to track " << i << ": " << distance);
        if ( min_dist > distance ) {
            min_dist = distance;
            float d =  detail::side( p_v, layout->_track_starts[i], layout->_track_ends[i] );
            float d_left = detail::side( layout->_track_left_point[i], layout->_track_starts[i], layout->_track_ends[i] );

            if ( std::signbit(d) == std::signbit(d_left) ) // both on the left
                sign = 1.0;
            else
                sign = -1.0;
        }
    }
    LPT_DEBUG("agv_external", "e(" << p_v[0] << ", " << p_v[1] << ") = " << min_dist*sign );
    return fs0::make_object(sign*min_dist);
}
