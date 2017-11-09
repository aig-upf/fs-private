#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/state.hxx>
#include <tuple>
#include <memory>
#include <string>

namespace fs0 { namespace hybrid {

    class G0Heuristic {

    public:

        typedef     std::vector<VariableIdx>    Subspace;

        G0Heuristic( std::shared_ptr<State> sG );
        G0Heuristic( const G0Heuristic& ) = delete;
        G0Heuristic( G0Heuristic&& o );
        ~G0Heuristic() = default;

        //! The actual evaluation of the heuristic value for any given non-relaxed state s.
        unsigned evaluate(const State& state) const;

        void print_JSON( std::string filename ) const;

        static std::shared_ptr<G0Heuristic>
        create_relative_to_goal( );

    protected:
        std::vector< Subspace >                     _scopes;
        std::vector< std::vector<float> >           _breakpoints;
        std::shared_ptr<State>                      _sG;
    };

} }
