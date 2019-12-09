
#include <fs/core/models/sdd_lifted_state_model.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/actions/actions.hxx>

#include <fs/core/languages/fstrips/language.hxx>
#include <utility>
#include "utils.hxx"

namespace fs0 {

    State SDDLiftedStateModel::init() const {
        // We need to make a copy so that we can return it as non-const.
        // Ugly, but this way we make it fit the search engine interface without further changes,
        // and this is only called once per search.
        return State(_task.getInitialState());
    }

    bool SDDLiftedStateModel::goal(const State& state) const {
        return _task.getGoalSatManager().satisfied(state);
    }


    State SDDLiftedStateModel::next(const State& state, const SchematicActionID& action) const {
        return state;
//        auto ground_action = action.generate();
//        auto s1 = next(state, *ground_action);
//        delete ground_action;
//        return s1;
    }



    SDDActionIterator SDDLiftedStateModel::applicable_actions(const State& state) const {
        return {state, sdds_, _task.get_tuple_index()};
    }

    SDDActionIterator SDDLiftedStateModel::applicable_actions(const State& state, bool enforce_state_constraints) const {
        // We know (see constructor) that there are no state constraints
        return applicable_actions(state);
    }


    bool
    SDDLiftedStateModel::goal(const StateT& s, unsigned i) const {
        Binding binding;
        return _subgoals.at(i)->interpret(s, binding);
// 	return s.contains(_subgoals.at(i)); // TODO SHOULD BE:
        // const Atom& subgoal = _subgoals.at(i);
        // return s.check(subgoal.getVariable(), s.getValue());
    }

    SDDLiftedStateModel
    SDDLiftedStateModel::build(const Problem& problem) {
        const ProblemInfo& info = ProblemInfo::getInstance();
        auto sdds = load_sdds_from_disk(problem.getActionData(), info.getDataDir() + "/sdd");
        auto model = SDDLiftedStateModel(problem, sdds, obtain_goal_atoms(problem.getGoalConditions()));
        return model;
    }

    SDDLiftedStateModel::SDDLiftedStateModel(const Problem& problem, std::vector<std::shared_ptr<ActionSchemaSDD>> sdds, std::vector<const fs::Formula*> subgoals) :
            _task(problem),
            sdds_(std::move(sdds)),
            _subgoals(std::move(subgoals))
    {
        // At the moment we just ignore the state constraints. TODO We should do better error handling,
        // but all of this state constraint code is bound to be refactored soon.
//        const auto state_constraints = _task.getStateConstraints();
//        if (!state_constraints.empty()
//            || state_constraints.size() != 1
//            || !state_constraints[0]->is_tautology()
//        ) {
//            std::cout << "Couldn't handle state constraint " << *state_constraints[0] << std::endl;
//            throw std::runtime_error("Cannot enforce state constraints ATM with the SDD action iterator");
//        }
    }

} // namespaces
