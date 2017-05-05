#include <dynamics/active_ode_set.hxx>

#include <iostream>

#include <problem.hxx>
#include <state.hxx>
#include <applicability/action_managers.hxx>
#include <languages/fstrips/builtin.hxx>
#include <dynamics/integrator.hxx>
#include <utils/config.hxx>

#include <lapkt/tools/logging.hxx>

namespace fs0 { namespace dynamics {

    namespace fs = fs0::language::fstrips;

    ActiveODESet::ActiveODESet( const State& s )
        : _state( s ),  _hash(0), _ready(false)
    {
        const Problem& problem = Problem::getInstance();
        NaiveApplicabilityManager manager(problem.getStateConstraints());

        //! Collect processes
        for ( auto a : problem.getGroundActions() ) {
            if ( manager.isActive(s, *a) )
                _signature.push_back(a->getId());
        }
        //! Compute hash
        boost::hash_combine( _hash, boost::hash_value(  _signature ) );
    }

    State
    ActiveODESet::predictNextState( const State& s, const integrators::Integrator& I, double delta_time, double factor ) const {
        setup();
        State next(s);
        I( s, _rates_of_change, next, delta_time, factor);
        return next;
    }

    void
    ActiveODESet::setup() const {
        if (_ready ) return;
        // Collect affected variables and effects
        const Problem& problem = Problem::getInstance();
        LPT_DEBUG( "dynamics", "Active processes:" );
        for ( auto pid : _signature ) {
            auto proc = problem.getGroundActions()[pid];

            LPT_DEBUG( "dynamics","\t" << *proc );
            for ( auto eff : proc->getEffects() ) {
                const fs::StateVariable* var = dynamic_cast<const fs::StateVariable*>( eff->lhs() );
                if ( var == nullptr ) {
                    throw std::runtime_error("Symbol on left hand side of process '" + proc->getName() +  "' is not a State Variable!");
                }
                const fs::BinaryArithmeticTerm* rhs = dynamic_cast<const fs::BinaryArithmeticTerm*>(eff->rhs());
                if ( rhs == nullptr ) {
                    throw std::runtime_error("Right hand side of process '" + proc->getName() +  "' is not a binary arithmetic term!");
                }
                VariableIdx affected = var->getValue();
                auto eq = [affected]( const DifferentialEquation& eq ) { return eq._affected == affected; };
                auto it = std::find_if( _rates_of_change.begin(), _rates_of_change.end(), eq );
                if ( it == _rates_of_change.end() ) {

                    DifferentialEquation equation(affected);
                    equation._terms.push_back( rhs->getSubterms()[1] );
                    if ( dynamic_cast<const fs::AdditionTerm*>(rhs))
                        equation._signs.push_back( 1.0 );
                    else
                        equation._signs.push_back( -1.0 );
                    _rates_of_change.push_back( equation );
                    continue;
                }

                it->_terms.push_back( rhs->getSubterms()[1] );
                if ( dynamic_cast<const fs::AdditionTerm*>(rhs))
                    it->_signs.push_back( 1.0 );
                else
                    it->_signs.push_back( -1.0 );
            }
        }
        _ready = true;
    }

    std::ostream&
    ActiveODESet::print(std::ostream& os) const {
        os << "Current dynamics:" << std::endl;
        for ( auto eq: _rates_of_change ) {
            os << eq << std::endl;
        }

        return os;
    }


}}
