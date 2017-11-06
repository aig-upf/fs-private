
#pragma once


#include <fs_types.hxx>
#include <atom.hxx>
#include <languages/fstrips/formulae.hxx>

#include <vector>
#include <set>
#include <boost/container/flat_map.hpp>

#include <modules/hybrid/fs_soplex.hxx>

namespace fs0 { namespace spx {

    namespace fs = fs0::language::fstrips;

    /** A simple class that encapsulates a soplex linear program
        and allows to recover the solution
    */
    class LinearProgram {
    public:
        typedef int     LPVar;


        LinearProgram();
        LinearProgram(const LinearProgram&) = delete;

        void        add_constraint( const fs::Formula* f );

        void        setup();
        void        solve();

        int         num_vars() const { return _num_vars; }

        const  std::vector<Atom>&
                    solution() const { return _solution; }

        virtual     ~LinearProgram();

    protected:

        soplex::SoPlex                                              _lp;
        std::vector<Atom>                                           _solution;
        std::set<VariableIdx>                                       _variables;
        int                                                         _num_vars;
        std::vector< const fs::RelationalFormula* >                 _constraints;
        boost::container::flat_map< VariableIdx, LPVar >            _fs_var_to_lp;
        boost::container::flat_map< LPVar, VariableIdx >            _lp_var_to_fs;

    };

}}
