#pragma once

#include <state.hxx>
#include <atom.hxx>
#include <actions/actions.hxx>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace fs0 {
    class Problem;

    namespace dynamics {

        class WaitAction : public ProceduralAction {
        public:
            typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS > Graph;
            typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

            WaitAction(unsigned id, const ActionData& action_data);
            virtual ~WaitAction() = default;

            virtual void apply( const State& s, std::vector<Atom>& atoms ) const override;

            static  const WaitAction* create(Problem& p);

        protected:
            void compute_reactions_causal_graph(const State& s) const;
            void process_events( const State& s, std::vector<Atom>& atoms, bool do_zcc ) const;

        protected:

            VariableIdx                                         _clock_var;
            std::vector< const GroundAction* >                  _exogenous;
            mutable std::vector< std::vector< std::pair<Vertex, const GroundAction*> > >   _reaction_stages;
        };
}} // namespaces
