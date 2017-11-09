#include <fs/hybrid/dynamics/registry.hxx>
#include <fs/hybrid/dynamics/explicit_euler.hxx>
#include <fs/hybrid/dynamics/implicit_euler.hxx>
#include <fs/hybrid/dynamics/runge_kutta_2.hxx>
#include <fs/hybrid/dynamics/runge_kutta_4.hxx>

namespace fs0 { namespace dynamics { namespace integrators {

    IntegratorRegistry& IntegratorRegistry::instance() {
        static IntegratorRegistry theInstance;

        return theInstance;
    }

    IntegratorRegistry::IntegratorRegistry() {
        registerIntegratorCreators();
    }

    void
    IntegratorRegistry::addFactoryMethod( Config::IntegratorT type, IntegratorCreator m) {
        _factory_methods.insert( std::make_pair( type, m ));
    }

    Integrator::sptr
    IntegratorRegistry::instantiate( Config::IntegratorT type ) const {
        auto it = _factory_methods.find(type);
        assert (it != _factory_methods.end());
        return it->second();
    }

    void
    IntegratorRegistry::registerIntegratorCreators() {
            addFactoryMethod( Config::IntegratorT::ExplicitEuler, []() { return std::make_shared<ExplicitEuler>(); } );
            addFactoryMethod( Config::IntegratorT::RungeKutta2, []() { return std::make_shared<RungeKutta2>(); } );
            addFactoryMethod( Config::IntegratorT::RungeKutta4, []() { return std::make_shared<RungeKutta4>(); } );
            addFactoryMethod( Config::IntegratorT::ImplicitEuler, []() { return std::make_shared<ImplicitEuler>(); } );
    }

}}}
