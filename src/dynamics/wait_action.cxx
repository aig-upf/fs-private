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
#include <tuple>
#include <set>

#include <boost/graph/topological_sort.hpp>


namespace fs = fs0::language::fstrips;

namespace fs0 { namespace dynamics {

    typedef std::set<VariableIdx> VariableSet;


    WaitAction::WaitAction(unsigned id, const ActionData& action_data)
        : ProceduralAction(id, action_data, Binding::EMPTY_BINDING) {
        _clock_var = ProblemInfo::getInstance().getVariableId("clock_time()");
    }

    void
    WaitAction::apply( const State& s, std::vector<Atom>& atoms ) const {
        if ( Config::instance().hasHorizon() && (fs0::value<float>(s.getValue(_clock_var)) > Config::instance().getHorizonTime()) )
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

        // Events
        // ======
        //
        // One-off, simultaneous, no superposition policy:
        if ( !_exogenous.empty() )
            process_events( s, atoms, do_zcc );
        #ifdef DEBUG
        // Compute and report change set
        LPT_DEBUG("dynamics", "Events Firing: Changed atoms: ");
        for ( Atom a : atoms ) {
            if ( a.getValue() == s.getValue(a.getVariable())) continue;
            object_id old_value = s.getValue(a.getVariable());
            object_id new_value = a.getValue();
            LPT_DEBUG( "dynamics", "\t" << info.getVariableName(a.getVariable()) << ": " << old_value << " -> " << new_value );
        }
        #endif

        while ( t < dt ) {
            State s_k( s, atoms );
            State s_k_plus_1 = odes.predictNextState( s_k, *integrator, h, 1.0 );
            // MRJ: Check state constraints
            if ( do_zcc && !manager.checkStateConstraints(s_k_plus_1) ) {
                // At least one state constraint was violated!
                LPT_DEBUG("dynamics", "WaitAction::apply: State constraints violated at lattice point: " << k );
                //atoms.clear();
                return; // we're done here
            }
            double t_k_plus_1 = fs0::value<float>(s_k_plus_1.getValue(_clock_var));
            if (  Config::instance().hasHorizon() && (t_k_plus_1 > Config::instance().getHorizonTime() )) {
                atoms.clear();
                LPT_DEBUG("dynamics", "Time out! t=" << t_k_plus_1 << " H=" << Config::instance().getHorizonTime() );
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

        // Events
        // ======
        //
        // One-off, simultaneous, no superposition policy:
        //if ( !_exogenous.empty() )
        //    process_events( s, atoms, do_zcc );

        #ifdef DEBUG
        // Compute and report change set
        LPT_DEBUG("dynamics", "Changed atoms: s -> s'");
        for ( Atom a : atoms ) {
            if ( a.getValue() == s.getValue(a.getVariable())) continue;
            object_id old_value = s.getValue(a.getVariable());
            object_id new_value = a.getValue();
            LPT_DEBUG( "dynamics", "\t" << info.getVariableName(a.getVariable()) << ": " << old_value << " -> " << new_value );
        }
        #endif

    }

    void
    WaitAction::compute_reactions_causal_graph(const State& s) const {
        _reaction_stages.clear();
        NaiveApplicabilityManager       manager(Problem::getInstance().getStateConstraints());
        std::vector<VariableSet> evt_precs(_exogenous.size());
        std::vector<VariableSet> evt_effs(_exogenous.size());
        Graph G(_exogenous.size());
        //std::vector<std::pair<unsigned, unsigned>> edges;
        for ( unsigned i = 0; i < _exogenous.size(); i++ ) {
            const GroundAction& e_i = *_exogenous[i];
            if (!manager.checkFormulaHolds(e_i.getPrecondition(), s)) continue;
            for ( auto eff : e_i.getEffects() )
                evt_effs[i].insert( fs::interpret_variable(*(eff->lhs()), s) );
            for ( unsigned j = 0; j < _exogenous.size(); j++ ){
                if (i == j ) continue;

                const GroundAction& e_j = *_exogenous[j];
                if (!manager.checkFormulaHolds(e_j.getPrecondition(), s)) continue;
                // MRJ: there's an edge (i,j) if the lhs of the effects of
                // event e_i appear on the precondition and rhs of the
                // effects in e_j.
                //for ( auto x : fs::all_state_variables(*e_j.getPrecondition()))
                //    evt_precs[j].insert(x->getValue());
                for (auto eff: e_j.getEffects() )
                    for ( auto x : fs::all_state_variables(*eff->rhs()) )
                        evt_precs[j].insert(x->getValue());

                for ( auto y : evt_effs[i] )
                    if (evt_precs[j].find(y) != evt_precs[j].end()) {
                        LPT_DEBUG("dynamics", e_i << " -> " << e_j);
                        boost::add_edge(i,j, G);
                        break;
                    }
            }
        }


        typedef std::vector< Vertex > container;
        container  c;
        topological_sort(G, std::back_inserter(c));
        _reaction_stages.clear();
        LPT_DEBUG("dynamics", "Reactions causal graph:");
        unsigned current_layer = 0;
        _reaction_stages.push_back( std::vector<std::pair<Vertex, const GroundAction*>>() );
        for ( container::reverse_iterator ii=c.rbegin(); ii!=c.rend(); ++ii) {
            const GroundAction& e_k = * _exogenous[*ii];
            if (!manager.checkFormulaHolds(e_k.getPrecondition(), s)) continue;
            if ( _reaction_stages[current_layer].size() > 0 ) {
                // if dependent, push into next layer
                bool dependant = false;
                for ( const auto& entry : _reaction_stages[current_layer]) {
                    for ( auto y : evt_effs[*ii] )
                        if (evt_precs[entry.first].find(y) != evt_precs[entry.first].end() ) {
                            dependant = true;
                            break;
                        }
                    if (dependant) break;
                }
                if (dependant) {
                    _reaction_stages.push_back( std::vector< std::pair<Vertex, const GroundAction*> >() );
                    current_layer++;
                    _reaction_stages[current_layer].push_back(std::make_pair(*ii,_exogenous[*ii]));
                    LPT_DEBUG( "dynamics", e_k);
                    LPT_DEBUG("dynamics", "\t Layer: " << current_layer );
                    continue;
                }
            }

            _reaction_stages[current_layer].push_back(std::make_pair(*ii,_exogenous[*ii]));
            LPT_DEBUG( "dynamics", e_k);
            LPT_DEBUG("dynamics", "\t Layer: " << current_layer );
        }
    }

    void
    WaitAction::process_events( const State& s, std::vector<Atom>& atoms, bool do_zcc ) const {
        const ProblemInfo& info = ProblemInfo::getInstance();
        NaiveApplicabilityManager       manager(Problem::getInstance().getStateConstraints());

        LPT_DEBUG("dynamics", "WaitAction::apply(): one-off cascading event activation");
        compute_reactions_causal_graph(s);
        unsigned events_fired = 0;
        State s_j(s, atoms);
        std::vector<Atom> A_j( atoms.begin(), atoms.end());
        std::map<VariableIdx, const GroundAction* > seen;
        LPT_DEBUG("dynamics", "\t One-off event activation support: " << s_j);

        for ( unsigned k = 0; k < _reaction_stages.size(); k++ ) {
            bool zc = false;
            for ( auto entry : _reaction_stages[k] ) {
                const GroundAction* a = entry.second;
                if (!manager.checkFormulaHolds(a->getPrecondition(), s_j)) continue;
                LPT_DEBUG("dynamics", "\t Event " << *a << " fired!");
                events_fired++;
                std::vector<Atom> eff_j = manager.computeEffects(s_j, *a);
                for ( Atom a_l : eff_j ) {
                    A_j[ a_l.getVariable() ] = a_l;
                    auto it = seen.find(a_l.getVariable());
                    if ( it != seen.end() ) {
                        std::stringstream buffer;
                        buffer << "Inconsistent semantics, revise model: " << std::endl;
                        buffer << "WaitAction::apply(): Events " << a->getName() << " and " << it->second->getName() << std::endl;
                        buffer << "firing simultaneously, affecting concurrently variable " << info.getVariableName(a_l.getVariable()) << std::endl;
                        LPT_INFO("dynamics", buffer.str());
                        throw std::runtime_error( buffer.str() );
                    }
                    seen.insert( std::make_pair(a_l.getVariable(), a));
                }
                // MRJ: decide what to do when we have state constraints
                if ( do_zcc ) {
                    State s_check( s, A_j );
                    if ( !manager.checkStateConstraints(s_check))  {
                        LPT_DEBUG("dynamics", "\t Application of event " << *a << " resulted in state violating state constraints!");
                        zc = true;
                        break;
                    }
                }
            }
            if (zc) break;
        }


        LPT_DEBUG("dynamics", "# Events fired: " << events_fired );
        atoms.assign( A_j.begin(), A_j.end() );
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


        WaitAction* wait = new WaitAction( ground_id, *adata );
        LPT_INFO("grounding", "Created wait action: " << *wait);
        for ( auto a : problem.getGroundActions() ) {
            if ( !a->isExogenous() )
                continue;
            wait->_exogenous.push_back( a );
        }
        LPT_INFO("grounding", "\t Events registered: " << wait->_exogenous.size() );

        return wait;
    }

}}
