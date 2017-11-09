#pragma once

#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <fs/core/fs_types.hxx>
#include <fs/core/actions/actions.hxx>
#include <iosfwd>

namespace fs0 { namespace dynamics {

    namespace fstrips = language::fstrips;

    class DifferentialEquation {
    public:
        DifferentialEquation( VariableIdx x, const GroundAction* proc ) : _affected(x), _proc(proc) {

        }


        DifferentialEquation( const DifferentialEquation& other ) {
            _affected = other._affected;
            _proc = other._proc;
            _context = other._context;
            _terms = other._terms;
            _signs = other._signs;
        }



        DifferentialEquation( DifferentialEquation&& other ) {
            _affected = other._affected;
            _proc = other._proc;
            _context = other._context;
            _terms = std::move(other._terms);
            _signs = std::move(other._signs);
        }

        DifferentialEquation&
        operator=(  DifferentialEquation&& other ) {
            if ( this == &other ) return *this;
            _affected = other._affected;
            _proc = other._proc;
            _context = other._context;
            _terms = std::move(other._terms);
            _signs = std::move(other._signs);
            return *this;
        }

        //! Prints a representation of the object to the given stream.
        friend std::ostream& operator<<(std::ostream &os, const DifferentialEquation&  entity) { return entity.print(os); }
        std::ostream& print(std::ostream& os) const;

        VariableIdx                             _affected;
        const GroundAction*                     _proc;
        std::vector<double>                     _signs;
        std::vector< const fs::Term* >          _terms;
        std::set< VariableIdx >                  _context;
    };



}} // namespaces
