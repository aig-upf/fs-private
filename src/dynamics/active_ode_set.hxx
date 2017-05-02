#pragma once

#include <memory>
#include <vector>
#include <memory>
#include <unordered_set>
#include <cassert>
#include <iostream>
#include <functional>
#include <boost/functional/hash.hpp>

#include <lapkt/tools/logging.hxx>

#include <dynamics/ode.hxx>
#include <languages/fstrips/builtin.hxx>
#include <dynamics/integrator.hxx>
#include <actions/actions.hxx>
#include <utils/config.hxx>
#include <state.hxx>


namespace fs0 {


    namespace dynamics {

    using fs = fs0::language::fstrips;


    template <typename StateModel>
    class ActiveODESet {
    public:

        typedef std::shared_ptr< ActiveODESet >     sptr;

        ActiveODESet( const State& s, const StateModel& model )
            : _state( s ), _model( model ), _hash(0), _ready(false)
        {
            //! Collect processes
            for ( auto id : _model.applicable_actions(s) ) {
                if ( _model.getTask().getGroundActions()[id]->isNatural() )
                    _signature.push_back(pid);
            }
            //! Compute hash
            boost::hash_combine( _hash, boost::hash_value(  _signature ) );
        }

        State
        predictNextState( const State& s, const integrators::Integrator& I, double delta_time ) const {
            setup();
            State next(s);
            I( s, _rates_of_change, next, delta_time);
            return next;
        }

        ~ActiveODESet() {

        }

        //! Prints a representation of the object to the given stream.
        friend std::ostream& operator<<(std::ostream &os, const ActiveODESet&  entity) { return entity.print(os); }
        std::ostream& print(std::ostream& os) const {
            os << "Current dynamics:" << std::endl;
            for ( auto eq: _rates_of_change ) {
                os << eq << std::endl;
            }

            return os;
        }

        std::size_t     hash() const { return _hash; }

        const State&                                    _state;
        const StateModel&                               _model;
        std::size_t                                     _hash;
        mutable bool                                    _ready;

        std::vector< GroundAction::IdType >             _signature;
        mutable std::vector< DifferentialEquation >     _rates_of_change;

    protected:

        void setup() const {
            if (_ready ) return;
            // Collect affected variables and effects
            for ( auto pid : _signature ) {
                auto proc = _model.getTask().getGroundProcesses()[pid];
                LPT_DEBUG( "dynamics", "Active processes:" );
                LPT_DEBUG( "dynamics", *proc );
                for ( auto eff : proc->getEffects() ) {
                    VariableIdx affected = eff->lhs()->interpretVariable(_state);
                    auto eq = [affected]( const DifferentialEquation& eq ) { return eq._affected == affected; };
                    auto it = std::find_if( _rates_of_change.begin(), _rates_of_change.end(), eq );
                    if ( it == _rates_of_change.end() ) {
                        assert( isAdditionTerm(eff->rhs()) || isSubtractionTerm(eff->rhs()));
                        DifferentialEquation equation(affected);
                        equation._terms.push_back( eff->rhs()->getSubterms()[1] );
                        equation._signs.push_back( ( isAdditionTerm(eff->rhs()) ? 1.0 : -1.0) );
                        _rates_of_change.push_back( equation );
                        continue;
                    }
                    assert( isAdditionTerm(eff->rhs()) || isSubtractionTerm(eff->rhs()));
                    it->_terms.push_back(eff->rhs()->getSubterms()[1]);
                    it->_signs.push_back( ( isAdditionTerm(eff->rhs()) ? 1.0 : -1.0) );
                }
            }
            _ready = true;
        }
    };

    template <typename ModePtrT>
    struct mode_hash {
       size_t operator() (const ModePtrT& mode) const { return mode->hash(); }
    };

    template <typename ModePtrT>
    struct mode_equal_to {
       bool operator() (const ModePtrT& m1, const ModePtrT& m2) const { return m1->_signature == m2->_signature; }
    };


    template <typename ModePtrT>
    using mode_unordered_set = std::unordered_set<ModePtrT, mode_hash<ModePtrT>, mode_equal_to<ModePtrT>>;

    using mode_cache = mode_unordered_set< ActiveODESet::sptr >;

}} // namespaces
