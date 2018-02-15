#pragma once


#include <vector>
#include <valarray>
#include <unordered_map>

#include <lapkt/tools/logging.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/utils/external.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/state.hxx>


namespace fs = fs0::language::fstrips;
using namespace fs0;

typedef     std::unordered_map< object_id, VariableIdx >    VariableSet;
typedef     std::unordered_map< object_id, object_id >      ValueSet;
typedef     std::valarray<float>                            fvec;

class TrackLayout {
public:
    std::unordered_map< std::string, unsigned >     _tracks;
    std::vector< fvec >                             _track_starts;
    std::vector< fvec >                             _track_ends;
    std::vector< fvec >                             _track_left_point;
};

class External : public fs0::ExternalI {

public:

    External( const ProblemInfo& info, const std::string& data_dir );
    ~External();

    void registerComponents() const;

    std::shared_ptr<TrackLayout> resolve_layout( object_id layout ) const;

public:
    std::vector<object_id>                          _vehicle;

    VariableSet                                     _x;
    VariableSet                                     _y;
    VariableIdx                                     _track_layout;

    mutable std::map< object_id, std::shared_ptr<TrackLayout> > _layouts;

    std::string                                     _data_dir;

};

class TrackDistance : public fs::AxiomaticTerm {
public:
    TrackDistance( const std::vector<const fs::Term*>& subterms );
    TrackDistance* clone(const std::vector<const fs::Term*>& subterms) const override;
    std::string name() const override { return "track_distance";}
    object_id compute(const State& state, std::vector<object_id>& arguments) const override;
protected:

    const External& _external;
};
