#include <dynamics/wait_action.hxx>
#include <utils/config.hxx>
#include <dynamics/active_ode_set.hxx>
#include <dynamics/registry.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <applicability/action_managers.hxx>
#include <lapkt/tools/logging.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/operations.hxx>

namespace fs = fs0::language::fstrips;


namespace fs0 { namespace dynamics {

    WaitAction::WaitAction(unsigned id, const ActionData& action_data)
        : ProceduralAction(id, action_data, Binding::EMPTY_BINDING) {
        std::cout << getName() << std::endl;
        _clock_var = ProblemInfo::getInstance().getVariableId("clock_time()");
    }

    void
    WaitAction::apply( const State& s, std::vector<Atom>& atoms ) const {
        if ( Config::instance().hasHorizon() && (boost::get<float>(s.getValue(_clock_var)) > Config::instance().getHorizonTime()) )
            throw std::runtime_error("WaitAction::apply: reached state beyond horizon");

        // MRJ: We collect the reference to the integrator selected by
        // the configuration
        auto integrator =
                integrators::IntegratorRegistry::instance().instantiate( Config::instance().getSuccessorPredictionType() );

        // MRJ: we compute the actual time step of the numeric integrator
        // by dividing dt (the discretization step) by the integration factor
        // (the number of intermediate points used for zero crossing control
        // and additional accuracy).
        double dt = Config::instance().getDiscretizationStep();
        double h = dt / Config::instance().getIntegrationFactor();
        bool do_zcc = Config::instance().getZeroCrossingControl();

        ActiveODESet                    odes(s);
        NaiveApplicabilityManager       manager(Problem::getInstance().getStateConstraints());

        const ProblemInfo& info = ProblemInfo::getInstance();
        double t = 0.0; // position in the interval
        unsigned k = 0; // index of point in lattice
        atoms.clear();
        for ( unsigned x = 0; x < info.getNumVariables(); x++ )
            atoms.push_back( Atom(x,s.getValue(x)));
        LPT_DEBUG("dynamics", "Input State: " << s);
        while ( t < dt ) {
            State s_k( s, atoms );
            State s_k_plus_1 = odes.predictNextState( s_k, *integrator, h, 1.0 );
            // MRJ: Check state constraints
            if ( do_zcc && !manager.checkStateConstraints(s_k_plus_1) ) {
                // At least one state constraint was violated!
                LPT_DEBUG("dynamics", "WaitAction::apply: State constraints violated at lattice point: " << k );
                atoms.clear();
                return; // we're done here
            }
            double t_k_plus_1 = boost::get<float>(s_k_plus_1.getValue(_clock_var));
            if (  Config::instance().hasHorizon() && (t_k_plus_1 > Config::instance().getHorizonTime() )) {
                atoms.clear();
                LPT_DEBUG("dynamics", "Time out!");
                return; // we're done as well, time is implicitly bounded
            }
            if ( t_k_plus_1 < -1e-2) {
                LPT_DEBUG("dynamics", "WaitAction::apply : Negative clock_time()!");
                LPT_DEBUG("dynamics", odes );
                throw std::runtime_error("WaitAction::apply: Time doesn't flow from the future towards the past");
            }
            // Atoms update
            LPT_DEBUG("dynamics", "Resulting state: " << s_k_plus_1 );
            for ( unsigned j = 0; j < atoms.size(); j++ )
                atoms[j] = Atom( atoms[j].getVariable(),  s_k_plus_1.getValue( atoms[j].getVariable() ) );
            t += h;
            k++;
        }

    }

    const WaitAction*
    WaitAction::create(Problem& problem) {

        unsigned wait_action_id = problem.getActionData().back()->getId() + 1;

        // MRJ: wait action is parametrized by dt, has no signature, parameters, or binding unit, precondition is trivial
        // and effects are procedurally defined
        ActionData* adata = new ActionData(wait_action_id, "wait(dt)", {}, {}, fs::BindingUnit({},{}), new fs::Tautology, {}, ActionData::Type::Control);
        problem.addActionData(adata);
        unsigned ground_id = 0;
        if ( !problem.getGroundActions().empty() ) {
            ground_id = problem.getGroundActions().back()->getId() + 1;
        }

        const WaitAction* wait = new WaitAction( ground_id, *adata );
        LPT_INFO("grounding", "Created wait action: " << *wait);
        return wait;
    }

}}
