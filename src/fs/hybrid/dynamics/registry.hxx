#pragma once

#include <fs/hybrid/dynamics/integrator.hxx>
#include <fs/core/utils//config.hxx>

namespace fs0 { namespace dynamics { namespace integrators {


    class IntegratorRegistry {

        typedef std::function<Integrator::sptr()> IntegratorCreator;

    public:

        static IntegratorRegistry& instance();

        Integrator::sptr    instantiate( Config::IntegratorT type ) const;

        void                addFactoryMethod( Config::IntegratorT type, IntegratorCreator method );

    protected:

        IntegratorRegistry();

        void registerIntegratorCreators();

        std::map<Config::IntegratorT, IntegratorCreator> 	_factory_methods;

    };

}}}
