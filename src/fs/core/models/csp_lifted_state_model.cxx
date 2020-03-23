
#include <fs/core/models/csp_lifted_state_model.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/applicability/action_managers.hxx>
#include <fs/core/actions/actions.hxx>

#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/constraints/gecode/handlers/lifted_action_csp.hxx>
#include <fs/core/actions/grounding.hxx>
#include "utils.hxx"


namespace fs0 {

State CSPLiftedStateModel::init() const {
    // We need to make a copy so that we can return it as non-const.
    // Ugly, but this way we make it fit the search engine interface without further changes,
    // and this is only called once per search.
    return State(_task.getInitialState());
}

bool CSPLiftedStateModel::goal(const State& state) const {
    return _task.getGoalSatManager().satisfied(state);
}

bool CSPLiftedStateModel::is_applicable(const State& state, const ActionType& action, bool enforce_state_constraints) const {
    auto ground_action = action.generate();
    bool res = is_applicable(state, *ground_action, enforce_state_constraints);
    delete ground_action;
    return res;
}

bool CSPLiftedStateModel::is_applicable(const State& state, const GroundAction& action, bool enforce_state_constraints) const {
    NaiveApplicabilityManager manager(_task.getStateConstraints());
    return manager.isApplicable(state, action, enforce_state_constraints);
}

State CSPLiftedStateModel::next2(const State& state, const LiftedActionID& aid) const {
    auto binding = aid.get_full_binding();
    auto effects = ActionGrounder::bind_effects(aid.getActionData(), binding, ProblemInfo::getInstance());
    NaiveApplicabilityManager::computeEffects(state, effects, _effects_cache);
    State s1(state, _effects_cache); // Copy everything into the new state and apply the changeset
    for (const auto pointer:effects) delete pointer;
    return s1;
}


State CSPLiftedStateModel::next(const State& state, const LiftedActionID& aid) const {
    auto& adata = aid.getActionData();
    const auto& binding = aid.get_binding();
    assert(binding.is_complete());
    auto& op = lifted_operators_[adata.getId()];
    // Note that we don't need to check the precondition of the operator, only evaluate the effects:
    evaluate_simple_lifted_operator(state, op, binding, ProblemInfo::getInstance(), false, _effects_cache);
    return State(state, _effects_cache); // Copy everything into the new state and apply the changeset
}

gecode::CSPActionIterator CSPLiftedStateModel::applicable_actions(const State& state, bool enforce_state_constraints) const {
    if ( enforce_state_constraints )
        return gecode::CSPActionIterator(state, _handlers, _task.getStateConstraints(), _task.get_tuple_index());
    return gecode::CSPActionIterator(state, _handlers, {}, _task.get_tuple_index());
}


bool
CSPLiftedStateModel::goal(const StateT& s, unsigned i) const {
    Binding binding;
    return _subgoals.at(i)->interpret(s, binding);
// 	return s.contains(_subgoals.at(i)); // TODO SHOULD BE:
    // const Atom& subgoal = _subgoals.at(i);
    // return s.check(subgoal.getVariable(), s.getValue());
}

CSPLiftedStateModel
CSPLiftedStateModel::build(const Problem& problem) {
    auto model = CSPLiftedStateModel(problem, obtain_goal_atoms(problem.getGoalConditions()));
    model.set_handlers(gecode::LiftedActionCSP::create(problem.getPartiallyGroundedActions(), problem.get_tuple_index(), false, false));


    std::vector<SimpleLiftedOperator> ops;
    for (const auto a:problem.getPartiallyGroundedActions()) {
        ops.emplace_back(compile_schema_to_simple_lifted_operator(*a));
    }

    model.set_operators(std::move(ops));
    return model;
}

CSPLiftedStateModel::CSPLiftedStateModel(const Problem& problem, const std::vector<const fs::Formula*>& subgoals) :
        _task(problem),
        _subgoals(subgoals)
{}

} // namespaces
