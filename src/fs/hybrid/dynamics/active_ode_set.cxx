#include <fs/hybrid/dynamics/active_ode_set.hxx>

#include <iostream>

#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/applicability//action_managers.hxx>
#include <fs/core/languages/fstrips/builtin.hxx>
#include <fs/hybrid/dynamics/integrator.hxx>
#include <fs/core/utils//config.hxx>
#include <fs/core/languages/fstrips/operations.hxx>

#include <lapkt/tools/logging.hxx>
#include <tuple>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

namespace fs0 { namespace dynamics {

    namespace fs = fs0::language::fstrips;

    ActiveODESet::ActiveODESet( const State& s )
        : _state( s ),  _hash(0), _ready(false), _decompose_ode( Config::instance().getOption<bool>("dynamics.decompose_ode", false) )
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
        setup(s);

        State next(s);
        if ( _decompose_ode ) {
            auto computation_graph = extractComputationGraph();
            for ( const auto& layer : computation_graph )
                I( s, layer, next, delta_time, factor);
        } else
            I( s, _rates_of_change, next, delta_time, factor);
        next.updateHash();
        return next;
    }

    void
    ActiveODESet::setup(const State& state) const {
        if (_ready ) return;
        // Collect affected variables and effects
        const Problem& problem = Problem::getInstance();
        LPT_DEBUG( "dynamics", "Active processes:" );
        for ( auto pid : _signature ) {
            auto proc = problem.getGroundActions()[pid];

            LPT_DEBUG( "dynamics","\t" << *proc );
            for ( auto eff : proc->getEffects() ) {
                LPT_DEBUG( "dynamics", "\t\t" << *(eff->lhs()) << " := " << *(eff->rhs()));
                VariableIdx affected = fs::interpret_variable(*(eff->lhs()), state);
                const fs::BinaryArithmeticTerm* rhs = dynamic_cast<const fs::BinaryArithmeticTerm*>(eff->rhs());
                if ( rhs == nullptr ) {
                    throw std::runtime_error("Right hand side of process '" + proc->getName() +  "' is not a binary arithmetic term!");
                }

                auto eq = [affected]( const DifferentialEquation& eq ) { return eq._affected == affected; };
                auto it = std::find_if( _rates_of_change.begin(), _rates_of_change.end(), eq );
                if ( it == _rates_of_change.end() ) {

                    DifferentialEquation equation(affected, proc);
                    const fs::Term* expression = rhs->getSubterms()[1];
                    equation._terms.push_back( expression );
                    for ( auto x : fs::all_state_variables(*expression) )
                        equation._context.insert(x->getValue());

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

    std::vector< std::vector<DifferentialEquation> >
    ActiveODESet::extractComputationGraph( ) const {
        typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS > Graph;
        typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

        Graph G(_rates_of_change.size());
        //std::vector<std::pair<unsigned, unsigned>> edges;
        for ( unsigned i = 0; i < _rates_of_change.size(); i++ ) {
            const DifferentialEquation& f_i = _rates_of_change[i];
            for ( unsigned j = i + 1; j < _rates_of_change.size(); j++ ){
                const DifferentialEquation& f_j = _rates_of_change[j];
                // MRJ: both processes inside the same "box",
                // interdepencies are to be ignored
                if ( f_i._proc == f_j._proc ) continue;
                if (f_j._context.find(f_i._affected) != f_j._context.end() ) {
                    LPT_DEBUG("dynamics", f_i << " -> " << f_j);
                    boost::add_edge(i,j, G);
                }
                    //edges.push_back( std::make_pair(i,j));
                if (f_i._context.find(f_j._affected) != f_i._context.end() ) {
                    LPT_DEBUG("dynamics", f_j << " -> " << f_i);
                    boost::add_edge(j,i, G);
                }

                    //edges.push_back( std::make_pair(j,i));
            }
        }


        typedef std::vector< Vertex > container;
        container  c;
        topological_sort(G, std::back_inserter(c));
        std::vector< std::vector<DifferentialEquation> > topo_sorted_dependency_graph;
        LPT_DEBUG("dynamics", "Active ODE set computation graph:");
        unsigned current_layer = 0;
        topo_sorted_dependency_graph.push_back( std::vector<DifferentialEquation>() );
        for ( container::reverse_iterator ii=c.rbegin(); ii!=c.rend(); ++ii) {
            DifferentialEquation f_k =  _rates_of_change[*ii];
            LPT_DEBUG( "dynamics", f_k);
            if ( topo_sorted_dependency_graph[current_layer].size() > 0 ) {
                // if dependent, push into next layer
                bool dependant = false;
                for ( const auto& ode : topo_sorted_dependency_graph[current_layer])
                    if (f_k._context.find(ode._affected) != f_k._context.end()) {
                        dependant = true;
                        break;
                    }
                if (dependant) {
                    topo_sorted_dependency_graph.push_back( std::vector<DifferentialEquation>() );
                    current_layer++;
                    topo_sorted_dependency_graph[current_layer].push_back(f_k);
                    LPT_DEBUG("dynamics", "\t Layer: " << current_layer );
                    continue;
                }
            }

            topo_sorted_dependency_graph[current_layer].push_back(_rates_of_change[*ii]);
            LPT_DEBUG("dynamics", "\t Layer: " << current_layer );
        }

        return topo_sorted_dependency_graph;
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
