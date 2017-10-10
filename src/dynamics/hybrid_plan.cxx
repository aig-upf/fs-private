#include <dynamics/hybrid_plan.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <actions/action_id.hxx>
#include <applicability/formula_interpreter.hxx>
#include <applicability/action_managers.hxx>
#include <lapkt/tools/logging.hxx>
#include <actions/checker.hxx>

// For writing the traces
#include <utils/archive/json.hxx>

using namespace rapidjson;

namespace fs0 { namespace dynamics {

    HybridPlan::HybridPlan() {

    }

    HybridPlan::~HybridPlan() {

    }

    void
    HybridPlan::interpret_plan( const ActionPlan& plan) {

        return interpret_plan(Checker::transform(Problem::getInstance(), plan));
    }

    void
    HybridPlan::interpret_plan(const std::vector<LiftedActionID>& plan) {
        return interpret_plan(Checker::transform(Problem::getInstance(), plan));
    }


    void
    HybridPlan::interpret_plan( const std::vector<const GroundAction*>& plan ) {
        // Flush plan and trajectory
        _the_plan.clear();
        _trajectory.clear();

        // We need to execute the plan to get the time stamps of the
        // control and exogenous actions
        const ProblemInfo& info = ProblemInfo::getInstance();
        const Problem& problem = Problem::getInstance();

    	NaiveApplicabilityManager manager(problem.getStateConstraints());
    	// First we make sure that the whole plan is applicable
    	State state(problem.getInitialState());
        VariableIdx time_var_idx = info.getVariableId("clock_time()");
        unsigned control_count = 0, exo_count = 0;

    	for (const GroundAction* action : plan) {
    		if (!manager.isApplicable(state, *action, true)) {
                std::stringstream buffer;
                buffer << "HybridPlan::interpret_plan(): Plan is not valid (ground action " << action->getName() << " not applicable!)" << std::endl;
                buffer << "Current state: " << state << std::endl;
                buffer << "Initial state: " << problem.getInitialState() << std::endl;
                buffer << "Plan:" << std::endl;
                for ( auto a : plan )
                    buffer << "\t" << *a << std::endl;

                LPT_INFO("main", buffer.str());
                throw std::runtime_error(buffer.str() );
            }

            // Record time and action if it is not the wait action
            if ( action != problem.get_wait_action() ) {
                if (action->isControl())
                    control_count++;
                if (action->isExogenous())
                    exo_count++;
                _the_plan.push_back( std::make_tuple( fs0::value<float>(state.getValue(time_var_idx)), action) );
            }

    		state.accumulate(NaiveApplicabilityManager::computeEffects(state, *action)); // Accumulate the newly-produced atoms
    	}
        // MRJ: add dummy action at the very end of the plan
        _the_plan.push_back( std::make_tuple( fs0::value<float>(state.getValue(time_var_idx)), nullptr) );
        LPT_INFO("main", "HybridPlan::interpret_plan() : Events in hybrid plan: " << _the_plan.size() << " # control: " << control_count << " # exogenous: " << exo_count  );
        LPT_INFO("main", "HybridPlan::interpret_plan() : Duration: " << get_duration() << " time units");

        LPT_DEBUG("cout", "HybridPlan::interpret_plan() : Events in hybrid plan: " << _the_plan.size() << " # control: " << control_count << " # exogenous: " << exo_count  );
        LPT_DEBUG("cout", "HybridPlan::interpret_plan() : Duration: " << get_duration() << " time units");

        if ( !problem.getGoalSatManager().satisfied(state) )
            LPT_DEBUG("cout", "HybridPlan::interpret_plan(): WARNING: plan doesn't achieve the goal!");
    }

    void
    HybridPlan::save_simulation_settings(float time_step, unsigned npoints, Config::IntegratorT solver) {
        Config& cfg = Config::instance();
        auto cfg_integrator = cfg.getSuccessorPredictionType();
        auto cfg_dt = cfg.getDiscretizationStep();
        auto cfg_points = cfg.getIntegrationFactor();
        bool cfg_do_zcc = cfg.getZeroCrossingControl();
        _saved_settings = std::make_tuple( cfg_integrator, cfg_dt, cfg_points, cfg_do_zcc );
    }

    void
    HybridPlan::restore_simulation_settings() {
        Config::IntegratorT solver;
        float time_step;
        unsigned npoints;
        bool do_zcc;
        std::tie( solver, time_step, npoints, do_zcc ) = _saved_settings;

        // Restore original settings
        Config& cfg = Config::instance();
        cfg.setSuccessorPredictionType(solver);
        cfg.setDiscretizationStep(time_step);
        cfg.setIntegrationFactor(npoints);
        cfg.setZeroCrossingControl(do_zcc);
    }

    void
    HybridPlan::simulate( float time_step, float duration, unsigned npoints, Config::IntegratorT solver ) {
        save_simulation_settings( time_step, npoints, solver );

        Config& cfg = Config::instance();
        const Problem& problem = Problem::getInstance();
        const GroundAction* wait_action = problem.get_wait_action();
        const ProblemInfo& info = ProblemInfo::getInstance();
        VariableIdx time_var_idx = info.getVariableId("clock_time()");

        LPT_INFO( "simulation", "Starting plan Simulation");
        _trajectory.clear();

        NaiveApplicabilityManager manager(problem.getStateConstraints());
        auto s = std::make_shared<State>( problem.getInitialState() );
        _trajectory.push_back( s );
        if ( _the_plan.empty() ) {
            LPT_INFO( "simulation", "Plan is empty!");
            // Just simulate for the duration
            float H = duration;
            while ( H > 0.0 ) {
                float h = std::min((float)time_step, H );
                LPT_INFO( "simulation", "Integration step duration: " << h << " time units" );
                float old_step = cfg.getDiscretizationStep();
                cfg.setDiscretizationStep(h);

                auto tmp = std::make_shared<State>(*s);
                tmp->accumulate(NaiveApplicabilityManager::computeEffects(*s, *wait_action));

                cfg.setDiscretizationStep(old_step);

                s = tmp;
                _trajectory.push_back( tmp );
                H -=h;
            }

            restore_simulation_settings();
            return;
        }

        float t;
        const GroundAction* a;
        std::tie( t, a ) =_the_plan[0];
        float time_left = ( duration < 0.0 ? get_duration() : duration);
        LPT_INFO( "simulation", "Duration: " << time_left << " time units");

        if ( t > 0.0 ) { // There's some waiting before the first action in the plan
            float H = std::min( (float)time_left, t);

            LPT_INFO( "simulation", "Idle time: " << H << " time units" );
            while ( H > 0.0 ) {
                float h = std::min((float)time_step, H );
                LPT_INFO( "simulation", "Integration step duration: " << h << " time units" );
                float old_step = cfg.getDiscretizationStep();
                cfg.setDiscretizationStep(h);

                auto tmp = std::make_shared<State>(*s);
                tmp->accumulate(NaiveApplicabilityManager::computeEffects(*s, *wait_action));

                cfg.setDiscretizationStep(old_step);

                s = tmp;
                _trajectory.push_back( tmp );
                H -=h;
                time_left -= h;
            }
        } else {

        }

        if ( time_left < cfg.getDiscretizationStep() ) {
            for (unsigned i = 0; i < _the_plan.size(); i++) {
                std::tie( t, a ) =_the_plan[i];
                LPT_INFO( "simulation", "State: " << *s );

                if ( a == nullptr ) {
                    LPT_INFO("simulation", "Simulation finished, states in trajectory: " << _trajectory.size());
                    LPT_DEBUG( "cout", "HybridPlan::simulate() : Simulation Finished, states in trajectory: " << _trajectory.size() );
                    restore_simulation_settings();
                    return;
                }
                auto next = std::make_shared<State>(*s);
                next->accumulate(NaiveApplicabilityManager::computeEffects(*s, *a));
                _trajectory.push_back( next );
                s = next;
                LPT_INFO("simulation", "Action applied: " << *s );
            }
            LPT_INFO("simulation", "Simulation Finished, states in trajectory: " << _trajectory.size() );
            LPT_INFO("simulation", "Simulation finished, time left (" << time_left << ") less than discretisation step (" <<  cfg.getDiscretizationStep() << ")");
            restore_simulation_settings();
            return;
        }

    	for (unsigned i = 0; i < _the_plan.size(); i++) {
            std::tie( t, a ) =_the_plan[i];
            LPT_INFO( "simulation", "State: " << *s );

            if ( a == nullptr  ) {
                if (  time_left > cfg.getDiscretizationStep()) break;
                LPT_INFO("simulation", "Simulation finished, states in trajectory: " << _trajectory.size());
                LPT_DEBUG( "cout", "HybridPlan::simulate() : Simulation Finished, states in trajectory: " << _trajectory.size() );
                restore_simulation_settings();
                return;
            }
            auto next = std::make_shared<State>(*s);
            next->accumulate(NaiveApplicabilityManager::computeEffects(*s, *a));
            s = next;
            LPT_INFO("simulation", "Action applied: " << *s );
            float t2;
            if ( i < _the_plan.size() - 1 ) {
                // Numeric integration of dynamics
                const GroundAction* next_action;
                std::tie( t2, next_action ) =_the_plan[i+1];
            }
            else
                t2 = time_left;
            float H = std::min( t2 - t, time_left );

            LPT_INFO( "simulation", "Idle time: " << H << " time units" );
            while ( H > 0.0 ) {
                float h = std::min(time_step, H );
                LPT_INFO( "simulation", "Integration step duration: " << h << " time units" );
                float old_step = cfg.getDiscretizationStep();
                cfg.setDiscretizationStep(h);

                auto tmp = std::make_shared<State>(*s);
                tmp->accumulate(NaiveApplicabilityManager::computeEffects(*s, *wait_action));
                LPT_INFO("simulation", *s);
                cfg.setDiscretizationStep(old_step);

                s = tmp;
                _trajectory.push_back( tmp );
                H -=h;
                time_left -= h;
            }

            if ( time_left < cfg.getDiscretizationStep() ) {
                LPT_INFO("simulation", "Simulation finished, time left (" << time_left << ") less than discretisation step (" <<  cfg.getDiscretizationStep() << ")");
                LPT_INFO("simulation", "States in trajectory: " << _trajectory.size() );
                restore_simulation_settings();
                return;
            }
    	}
        LPT_INFO("simulation", "time left is: " << time_left << " units");
        if ( time_left > cfg.getDiscretizationStep() ) {
            LPT_INFO("simulation", "Simulation asked to run after plan end time");
            float H = time_left;

            LPT_INFO( "simulation", "Idle time: " << H << " time units" );
            while ( H > 0.0 ) {
                float h = std::min(time_step, H );
                LPT_INFO( "simulation", "Integration step duration: " << h << " time units" );
                float old_step = cfg.getDiscretizationStep();
                cfg.setDiscretizationStep(h);

                auto tmp = std::make_shared<State>(*s);
                tmp->accumulate(NaiveApplicabilityManager::computeEffects(*s, *wait_action));
                LPT_INFO("simulation", *s);
                cfg.setDiscretizationStep(old_step);

                s = tmp;
                _trajectory.push_back( tmp );
                H -=h;
                time_left -= h;
            }
        }

        LPT_INFO("simulation", "Simulation Finished, states in trajectory: " << _trajectory.size() );
        LPT_DEBUG( "cout", "HybridPlan::simulate() : Simulation Finished, states in trajectory: " << _trajectory.size() );
        restore_simulation_settings();
    }

    void
    HybridPlan::validate( float time_step, float duration, unsigned npoints, Config::IntegratorT solver ) {
        throw std::runtime_error("HybridPlan::validate() : Not Implemented Yet!");
    }

    void
    HybridPlan::save_simulation_trace( std::string filename ) {
        /* code */
        const ProblemInfo& info = ProblemInfo::getInstance();
        Document trace;
        Document::AllocatorType& allocator = trace.GetAllocator();
        trace.SetObject();
        Value domainName;
        domainName.SetString(StringRef(info.getDomainName().c_str()));
        trace.AddMember("domain", domainName.Move(), allocator );
        Value instanceName;
        instanceName.SetString(StringRef(info.getInstanceName().c_str()));
        trace.AddMember("instance", instanceName.Move(), allocator );


        Value duration( get_duration() );
        trace.AddMember("duration", duration, allocator );

        Value control_events(kArrayType);
        {
            for ( auto entry : _the_plan ) {
                float t;
                const GroundAction* a;
                std::tie(t,a) = entry;
                if ( a == nullptr) continue;
                if ( !a->isControl() ) continue;
                Value event(kObjectType);
                {
                    event.AddMember( "time", Value(t).Move(), allocator);
                    Value s;
                    s.SetString(StringRef(a->getName().c_str()));
                    event.AddMember( "name", s.Move(), allocator);
                }
                control_events.PushBack(event.Move(), allocator);
            }

        }
        trace.AddMember("control_events", control_events, allocator);

        Value exo_events(kArrayType);
        {
            for ( auto entry : _the_plan ) {
                float t;
                const GroundAction* a;
                std::tie(t,a) = entry;
                if ( a == nullptr) continue;
                if ( !a->isExogenous() ) continue;
                Value event(kObjectType);
                {
                    event.AddMember( "time", Value(t).Move(), allocator);
                    Value s;
                    s.SetString(StringRef(a->getName().c_str()));
                    event.AddMember( "name", s.Move(), allocator);
                }
                exo_events.PushBack(event.Move(), allocator);
            }

        }
        trace.AddMember("exo_events", exo_events, allocator);



        Value trajectory(kArrayType);
        {
            for ( auto s : _trajectory ) {
                Value state(kObjectType);
                JSONArchive::store( state, allocator, *s);
                trajectory.PushBack(state.Move(), allocator);
            }
        }
        trace.AddMember("trajectory", trajectory, allocator);

        FILE* fp = fopen(filename.c_str(), "wb"); // non-Windows use "w"
        char writeBuffer[65536];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        trace.Accept(writer);
        fclose(fp);
    }
}}
