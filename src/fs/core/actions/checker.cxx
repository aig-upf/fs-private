
#include <iostream>
#include <iomanip>

#include <fs/core/actions/checker.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/state.hxx>
#include <fs/core/utils/config.hxx>
#include <lapkt/tools/logging.hxx>

namespace fs0 {



bool Checker::check_correctness(const Problem& problem, const std::vector<const GroundAction*>& plan, const State& s0) {
	const Config& config = Config::instance();
	bool print_plan_trace = config.getOption<bool>("print_plan_trace", false);

	NaiveApplicabilityManager manager(problem.getStateConstraints());
	// First we make sure that the whole plan is applicable
	State state(s0);
    if (print_plan_trace) LPT_INFO("plan_trace", "s=" <<  state);
	for (const GroundAction* action:plan) {
		if (!manager.isApplicable(state, *action, true)) return false;
        if (print_plan_trace) LPT_INFO("plan_trace", "a=" <<  *action);
        state.update(NaiveApplicabilityManager::computeEffects(state, *action)); // Accumulate the newly-produced atoms
        if (print_plan_trace) LPT_INFO("plan_trace", "s=" <<  state);
	}

	// Now check that the resulting state is indeed a goal
	return problem.getGoalSatManager().satisfied(state);
}


std::vector<const GroundAction*> Checker::transform(const Problem& problem, const std::vector<LiftedActionID>& plan) {
	std::vector<const GroundAction*> transformed;

	// First we make sure that the whole plan is applicable
	for (const LiftedActionID& action_id:plan) {
		const GroundAction* action = action_id.generate();
		transformed.push_back(action);
	}
	return transformed;
}

std::vector<const GroundAction*> Checker::transform(const Problem& problem, const ActionPlan& plan) {
	std::vector<const GroundAction*> transformed;
	const auto& actions = problem.getGroundActions();

	for (unsigned idx:plan) {
		transformed.push_back(new GroundAction(*actions[idx]));
	}
	return transformed;
}


void Checker::debug_plan_execution(const Problem& problem, const std::vector<const GroundAction*>& plan, const State& s0) {
	NaiveApplicabilityManager manager(problem.getStateConstraints());

	std::size_t i = 0;
	State state(s0);

	std::cerr << std::setw(3) << i;
	std::cerr << ". " << state << std::endl;
	bool inapplicable = false;
	for (; i < plan.size(); ++i) {
		const GroundAction* action = plan[i];

//		std::cerr << std::setw(3) << i;
//		std::cerr << ". " << *action << std::endl;


		if (!manager.isApplicable(state, *action, true)) {
            std::cerr << std::endl << "ERROR: Action (" << i << ")"  << *action << " is not applicable to state (" << i << ")" << std::endl;
            inapplicable = true;
            break;
		}
        std::cerr << "\tAction (" << i << ") " << *action << " successfully applied to state (" << i << ")" << std::endl << std::endl;

        state.update(NaiveApplicabilityManager::computeEffects(state, *action)); // Accumulate the newly-produced atoms

		std::cerr << std::setw(3) << i + 1;
		std::cerr << ". " << state << std::endl;
	}

	// Now check that the resulting state is indeed a goal
	if (!inapplicable && !problem.getGoalSatManager().satisfied(state)) {
		std::cerr << "ERROR! The state that results from aplying the whole plan is NOT a goal state" << std::endl;
	}
}

void Checker::debug_plan_execution(const Problem& problem, const ActionPlan& plan, const State& s0) {
    auto transformed = transform(problem, plan);
    debug_plan_execution(problem, transformed, s0);
    for (auto x:transformed) delete x;
}

void Checker::debug_plan_execution(const Problem& problem, const std::vector<LiftedActionID>& plan, const State& s0) {
    auto transformed = transform(problem, plan);
    debug_plan_execution(problem, transformed, s0);
    for (auto x:transformed) delete x;
}

bool Checker::check_correctness(const Problem& problem, const ActionPlan& plan, const State& s0) {
    auto transformed = transform(problem, plan);
    auto res = check_correctness(problem, transformed, s0);
    for (auto x:transformed) delete x;
    return res;
}

bool Checker::check_correctness(const Problem& problem, const std::vector<LiftedActionID>& plan, const State& s0) {
    auto transformed = transform(problem, plan);
    auto res = check_correctness(problem, transformed, s0);
    for (auto x:transformed) delete x;
    return res;
}


} // namespaces
