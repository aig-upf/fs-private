#pragma once

#include <memory>
#include <vector>
#include <tuple>
#include <state.hxx>
#include <actions/actions.hxx>
#include <utils/config.hxx>

namespace fs0 {

    class LiftedActionID;

namespace dynamics {

    /**
        This class encapsulates a hybrid plan, decoupling its
        interpretation (as a hybrid automaton, a switched system, etc.)
        from the representation used to compute it.
    */
    class HybridPlan {
    protected:


    public:

        // Hybrid plans are made up of events sorted along the timeline,
        // events can be control or exogenous discrete transitions (actions)
        typedef     std::tuple< float, const GroundAction* >                        Event;
        typedef     std::vector< Event >                                            Timeline;
        typedef     std::tuple< Config::IntegratorT, double, unsigned, bool >       SimulatorSettings;


        HybridPlan();
        ~HybridPlan();

        void interpret_plan( const std::vector<const GroundAction*>& plan  );

        void interpret_plan(const ActionPlan& plan);

        void interpret_plan(const std::vector<LiftedActionID>& plan);

        float get_duration() const { return std::get<0>(_the_plan.back());}

        /**
            Simulates the evolution of the dynamical system described by
            the plan and the domain, numerically solving the sets of ODE's
            that hold at every time step.
        */
        void simulate( float time_step, float duration = -1.0, unsigned npoints = 1, Config::IntegratorT solver = Config::IntegratorT::ImplicitEuler );

        /**
            Like simulation but checking and offering a detailed report
            of violated constraints, preconditions, etc. on logs/validation.log
        */
        void validate( float time_step, float duration = -1.0, unsigned npoints = 1, Config::IntegratorT solver = Config::IntegratorT::ImplicitEuler );

        /**
            Saves simulation trace to a file in JSON format for visualization or
            further analysis.
        */
        void save_simulation_trace(std::string filename);

        const std::vector< std::shared_ptr<State> >& trajectory() const { return _trajectory; }

        std::vector<Event>  get_control_events() const {
            std::vector<Event> control_events;
            for ( auto evt : _the_plan )
                if ( std::get<1>(evt) != nullptr && std::get<1>(evt)->isControl() )
                    control_events.push_back(evt);
            return control_events;
        }

    protected:

        void save_simulation_settings(float time_step, unsigned npoints, Config::IntegratorT solver);
        void restore_simulation_settings();

        std::vector< std::shared_ptr<State> >       _trajectory;
        Timeline                                    _the_plan;
        SimulatorSettings                           _saved_settings;

    };


}}
