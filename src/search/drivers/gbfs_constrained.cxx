
#include <search/drivers/gbfs_constrained.hxx>
#include <constraints/direct/action_manager.hxx>
#include <search/drivers/validation.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/unreached_atom_rpg.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <constraints/gecode/handlers/ground_action_csp.hxx>
#include <constraints/gecode/handlers/ground_effect_csp.hxx>
#include <constraints/gecode/handlers/lifted_action_csp.hxx>
#include <actions/ground_action_iterator.hxx>
#include <languages/fstrips/formulae.hxx>
#include <utils/support.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace drivers {

template <typename GecodeHeuristic, typename DirectHeuristic>
std::unique_ptr<FS0SearchAlgorithm> GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::create(const Config& config, const GroundStateModel& model) const {
	const Problem& problem = model.getTask();
	const std::vector<const GroundAction*>& actions = problem.getGroundActions();
	
	bool novelty = config.useNoveltyConstraint();
	bool approximate = config.useApproximateActionResolution();
	bool delayed = config.getOption<bool>("search.delayed_evaluation");
	
	auto csp_type = decide_csp_type(problem);
	_unused(csp_type);
	assert(csp_type == Config::CSPManagerType::Gecode);
	
	LPT_INFO("main", "Chosen CSP Manager: Gecode");
	
	Validation::check_no_conditional_effects(problem);
	std::vector<std::shared_ptr<BaseActionCSP>> managers;
	if (Config::instance().getCSPModel() == Config::CSPModel::GroundedActionCSP) {
		managers = GroundActionCSP::create(actions, problem.get_tuple_index(), approximate, novelty);
		
	}  else if (Config::instance().getCSPModel() == Config::CSPModel::ActionSchemaCSP) {
		const std::vector<const PartiallyGroundedAction*>& base_actions = problem.getPartiallyGroundedActions();
		managers = LiftedActionCSP::create(base_actions, problem.get_tuple_index(), approximate, novelty);
		
	} else {
		throw std::runtime_error("Incompatible CSP model type");
	}
	
	const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), problem.getStateConstraints());
	ExtensionHandler extension_handler(problem.get_tuple_index(), managed);
	GecodeHeuristic heuristic(problem, problem.getGoalConditions(), problem.getStateConstraints(), std::move(managers), extension_handler);
	
	return std::unique_ptr<FS0SearchAlgorithm>(new aptk::StlBestFirstSearch<SearchNode, GecodeHeuristic, GroundStateModel>(model, std::move(heuristic), delayed));
}

template <typename GecodeHeuristic, typename DirectHeuristic>
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_csp_type(const Problem& problem) {
	
	if (Config::instance().getCSPManagerType() == Config::CSPManagerType::Gecode) return Config::CSPManagerType::Gecode;
	if (Config::instance().getCSPManagerType() == Config::CSPManagerType::ASP) return Config::CSPManagerType::ASP; // TODO - Probably we'll need to have some extra checks here

	auto type_required_by_actions = decide_action_manager_type(problem.getGroundActions());
	auto type_required_by_goal    = decide_builder_type(problem.getGoalConditions(), problem.getStateConstraints());
	
	if (Config::instance().getCSPManagerType() == Config::CSPManagerType::DirectIfPossible) {
		if (type_required_by_actions == Config::CSPManagerType::Direct && type_required_by_goal == Config::CSPManagerType::Direct) {
			return Config::CSPManagerType::Direct;
		} else {
			return Config::CSPManagerType::Gecode;
		}
	}
	
	// The type specified in the config file must be Direct
	assert(Config::instance().getCSPManagerType() == Config::CSPManagerType::Direct);
	if (type_required_by_actions == Config::CSPManagerType::Gecode || type_required_by_goal == Config::CSPManagerType::Gecode) {
		throw std::runtime_error("A 'Direct' CSP manager type was specified, but the problem requires a Gecode Manager");
	}
	return Config::CSPManagerType::Direct;
}

template <typename GecodeHeuristic, typename DirectHeuristic>
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_builder_type(const fs::Formula* goal_formula, const fs::Formula* state_constraints) {
	// ATM we simply check whether there are nested fluents within the formulae
	auto goal_conjunction = dynamic_cast<const fs::Conjunction*>(goal_formula);
	// If we have something other than a conjunction, then the gecode manager is required.
	if (!goal_conjunction || (!state_constraints->is_tautology() && !dynamic_cast<const fs::Conjunction*>(state_constraints))) return Config::CSPManagerType::Gecode;
	if (goal_formula->nestedness() > 0 || state_constraints->nestedness() > 0) return Config::CSPManagerType::Gecode;
	return Config::CSPManagerType::Direct;
}

template <typename GecodeHeuristic, typename DirectHeuristic>
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_action_manager_type(const std::vector<const GroundAction*>& actions) {
	if (Config::instance().getCSPManagerType() == Config::CSPManagerType::Gecode) return Config::CSPManagerType::Gecode;
	
	// If at least one action requires gecode, we'll use gecode throughout
	for (const auto action:actions) {
		if (!DirectActionManager::is_supported(*action)) return Config::CSPManagerType::Gecode;
	}
	return Config::CSPManagerType::Direct;
}

// explicit instantiations
template class GBFSConstrainedHeuristicsCreator<GecodeCRPG, DirectCRPG>;
template class GBFSConstrainedHeuristicsCreator<GecodeCHMax, DirectCHMax>;


} } // namespaces
