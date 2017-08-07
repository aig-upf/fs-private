#pragma once

#include <state.hxx>
#include <atom.hxx>
#include <actions/actions.hxx>

namespace fs0 {
    class Problem;

    namespace dynamics {
        class WaitAction : public ProceduralAction {
        public:
            WaitAction(unsigned id, const ActionData& action_data);
            virtual ~WaitAction() = default;

            virtual void apply( const State& s, std::vector<Atom>& atoms ) const override;

            static  const WaitAction* create(Problem& p);

        protected:

            VariableIdx                             _clock_var;
            std::vector< const GroundAction* >      _exogenous;
        };
}} // namespaces
