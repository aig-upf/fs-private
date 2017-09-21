#pragma once

#include <memory>
#include <vector>
#include <memory>
#include <unordered_set>
#include <cassert>
#include <iosfwd>
#include <functional>
#include <boost/functional/hash.hpp>


#include <dynamics/ode.hxx>
#include <actions/actions.hxx>


namespace fs0 {

    class Problem;

    namespace dynamics {

    namespace integrators { class Integrator; }

    class ActiveODESet {
    public:

        typedef std::shared_ptr< ActiveODESet >     sptr;

        ActiveODESet( const State& s );

        State
        predictNextState( const State& s, const integrators::Integrator& I, double delta_time, double factor ) const;

        std::vector< std::vector<DifferentialEquation> >
        extractComputationGraph(  ) const;

        ~ActiveODESet() = default;

        //! Prints a representation of the object to the given stream.
        friend std::ostream& operator<<(std::ostream &os, const ActiveODESet&  entity) { return entity.print(os); }
        std::ostream& print(std::ostream& os) const;

        std::size_t     hash() const { return _hash; }

        const State&                                    _state;
        std::size_t                                     _hash;
        mutable bool                                    _ready;
        bool                                            _decompose_ode;

        std::vector< GroundAction::IdType >             _signature;
        mutable std::vector< DifferentialEquation >     _rates_of_change;

    protected:

        void setup( const State& s) const;
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
