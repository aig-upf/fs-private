
#include <fs/core/models/simple_state_model.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/utils/system.hxx>
#include <fs/core/applicability/match_tree.hxx>
#include <lapkt/tools/logging.hxx>

#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/languages/fstrips/complex_existential_formula.hxx>

namespace fs0 {


//! A helper to derive the distinct goal atoms
/*
const std::vector<const fs::AtomicFormula*>&
obtain_goal_atoms(const fs::Formula* goal) {
	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	if (!conjunction) {
		throw std::runtime_error("This search mode can only be applied to problems featuring simple goal conjunctions");
	}
	return conjunction->getSubformulae();
}
*/


//! A helper to derive the distinct goal atoms
std::vector<const fs::Formula*>
obtain_goal_atoms(const Problem& problem, const fs::Formula* goal) {
	const auto* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
	const auto* ex_q = dynamic_cast<const fs::ExistentiallyQuantifiedFormula*>(goal);
	if (!conjunction && !ex_q) {
		throw std::runtime_error("This search mode can only be applied to problems featuring goal conjunctions of existentially-quantified formulas");
	}

	std::vector<const fs::Formula*> goal_atoms;

	if (ex_q) {
		// TODO This might be a memory leak
		goal_atoms.push_back(new fs::ComplexExistentialFormula(ex_q, problem.get_tuple_index()));
	} else {
		assert(conjunction);
		for (const fs::Formula* atom:conjunction->getSubformulae()) {
			goal_atoms.push_back(atom);
		}
	}

	return goal_atoms;
}


SimpleStateModel
SimpleStateModel::build(const Problem& problem) {
	return SimpleStateModel(problem, obtain_goal_atoms(problem, problem.getGoalConditions()));
}

SimpleStateModel::SimpleStateModel(const Problem& problem, const std::vector<const fs::Formula*>& subgoals) :
	_task(problem),
	_manager(build_action_manager(problem)),
	_subgoals(subgoals)
{}

SimpleStateModel::StateT
SimpleStateModel::init() const {
	// We need to make a copy so that we can return it as non-const.
	// This is only called once per search.
	return StateT(_task.getInitialState());
}

bool
SimpleStateModel::goal(const StateT& state) const {
	return _task.getGoalSatManager().satisfied(state);
}

bool
SimpleStateModel::is_applicable(const StateT& state, const ActionId& action, bool enforce_state_constraints) const {
	return is_applicable(state, *(_task.getGroundActions()[action]), enforce_state_constraints);
}

bool
SimpleStateModel::is_applicable(const StateT& state, const ActionType& action, bool enforce_state_constraints) const {
	return _manager->applicable(state, action, enforce_state_constraints);
}

SimpleStateModel::StateT
SimpleStateModel::next(const StateT& state, const GroundAction::IdType& actionIdx) const {
	return next(state, *(_task.getGroundActions()[actionIdx]));
}

SimpleStateModel::StateT
SimpleStateModel::next(const StateT& state, const GroundAction& a) const {
	a.apply(state,_effects_cache);
	StateT succ(state, _effects_cache); // Copy everything into the new state and apply the changeset
	LPT_EDEBUG("generated", "New state generated: " << succ);
	return succ;
}

bool
SimpleStateModel::goal(const StateT& s, unsigned i) const {
	Binding binding;
	return _subgoals.at(i)->interpret(s, binding);
// 	return s.contains(_subgoals.at(i)); // TODO SHOULD BE:
	// const Atom& subgoal = _subgoals.at(i);
	// return s.check(subgoal.getVariable(), s.getValue());
}


GroundApplicableSet
SimpleStateModel::applicable_actions(const StateT& state, bool enforce_state_constraints) const {
	return _manager->applicable(state, enforce_state_constraints);
}

ActionManagerI*
SimpleStateModel::build_action_manager(const Problem& problem) {
	using StrategyT = Config::SuccessorGenerationStrategy;
	const Config& config = Config::instance();
	const auto& actions = problem.getGroundActions();
	const auto& constraints = problem.getStateConstraints();
	const auto& tuple_idx =  problem.get_tuple_index();
	StrategyT strategy = config.getSuccessorGeneratorType();

	LPT_INFO( "main", "Ground actions: " << actions.size());

	if (strategy == StrategyT::adaptive) {
		// Choose match-tree if number of actions is large enough, otherwise naive.
		unsigned cutoff = config.getOption<unsigned>("mt_cutoff", 20000);
		if (actions.size() > cutoff) {
			strategy = StrategyT::match_tree;
			LPT_INFO("cout", "Chose Match-Tree as Successor Generator (" << actions.size() << " > " << cutoff << ")");

		} else {
			strategy = StrategyT::naive;
			LPT_INFO("cout", "Chose Naive as Successor Generator (" << actions.size() << " <= " << cutoff << ")");
		}
	}

	if (strategy == StrategyT::naive) {
		LPT_INFO( "cout", "Successor Generator: Naive");
		return new NaiveActionManager(actions, constraints);
	}

	if (strategy == StrategyT::functional_aware) {
		LPT_INFO( "cout", "Successor Generator: Functional Aware");
		BasicApplicabilityAnalyzer analyzer(actions, tuple_idx);
		analyzer.build();
		return new SmartActionManager(actions, constraints, tuple_idx, analyzer);


	} else if (strategy == StrategyT::match_tree) {
		const StateAtomIndexer& indexer = problem.getStateAtomIndexer();
		if (!indexer.is_fully_binary()) {
			throw std::runtime_error("Successor Generation Strategy: Match Tree: Variable domains not binary.");
		}
		LPT_INFO("cout", "Successor Generator: Match Tree");
		LPT_INFO("cout", "Mem. usage before match-tree construction: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");


		auto mng = new MatchTreeActionManager(actions, constraints, tuple_idx);
		LPT_INFO("cout", "Match-tree built with " << mng->count() << " nodes.");
		LPT_INFO("cout", "Mem. usage after match-tree construction: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
		return mng;
	}

	throw std::runtime_error("Unknown successor generation strategy");
}


} // namespaces
