#pragma once

#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/builtin.hxx>
#include <fs_types.hxx>
#include <iosfwd>

namespace fs0 { namespace dynamics {

    namespace fstrips = language::fstrips;

    class DifferentialEquation {
    public:
        DifferentialEquation( VariableIdx x ) : _affected(x) {

        }


        DifferentialEquation( const DifferentialEquation& other ) {
            _affected = other._affected;
            _terms = other._terms;
            _signs = other._signs;
        }



        DifferentialEquation( DifferentialEquation&& other ) {
            _affected = other._affected;
            _terms = std::move(other._terms);
            _signs = std::move(other._signs);
        }

        DifferentialEquation&
        operator=(  DifferentialEquation&& other ) {
            if ( this == &other ) return *this;
            _affected = other._affected;
            _terms = std::move(other._terms);
            _signs = std::move(other._signs);
            return *this;
        }

        //! Prints a representation of the object to the given stream.
        friend std::ostream& operator<<(std::ostream &os, const DifferentialEquation&  entity) { return entity.print(os); }
        std::ostream& print(std::ostream& os) const;

        VariableIdx                         _affected;
        std::vector<double>                 _signs;
        std::vector< const fs::Term* >  _terms;
    };



}} // namespaces
