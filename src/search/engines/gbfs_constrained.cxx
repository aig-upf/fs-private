
#include <search/engines/gbfs_constrained.hxx>
#include <problem.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/gecode_chmax.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <heuristics/relaxed_plan/direct_chmax.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace engines {

template <typename GecodeHeuristic, typename DirectHeuristic>
std::unique_ptr<FS0SearchAlgorithm> GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::create(const Config& config, const FS0StateModel& model) const {
	const Problem& problem = model.getTask();
	const std::vector<GroundAction::cptr>& actions = problem.getGroundActions();
	
	FS0SearchAlgorithm* engine = nullptr;
	if (decide_csp_type(problem) == Config::CSPManagerType::Gecode) {
		FINFO("main", "Chosen CSP Manager: Gecode");
		auto gecode_builder = GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints());
		
		std::vector<std::shared_ptr<GecodeActionManager>> managers;
		if (Config::instance().getCSPModel() == Config::CSPModel::ActionCSP) {
			managers = GecodeActionManager::createActionCSPs(actions);
		} else {
			managers = GecodeActionManager::createEffectCSPs(actions);
		}
		GecodeHeuristic gecode_builder_heuristic(model, std::move(managers), std::move(gecode_builder));
		engine = new aptk::StlBestFirstSearch<SearchNode, GecodeHeuristic, FS0StateModel>(model, std::move(gecode_builder_heuristic));
		
	} else {
		FINFO("main", "Chosen CSP Manager: Direct");
		auto direct_builder = DirectRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints());
		auto managers = DirectActionManager::create(actions);
		DirectHeuristic direct_builder_heuristic(model, std::move(managers), std::move(direct_builder));
		engine = new aptk::StlBestFirstSearch<SearchNode, DirectHeuristic, FS0StateModel>(model, std::move(direct_builder_heuristic));
	}
	
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

template <typename GecodeHeuristic, typename DirectHeuristic>
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_csp_type(const Problem& problem) {
	
	if (Config::instance().getCSPManagerType() == Config::CSPManagerType::Gecode) return Config::CSPManagerType::Gecode;


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
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_builder_type(const std::vector<fs::AtomicFormula::cptr>& goal_conditions, const std::vector<fs::AtomicFormula::cptr>& state_constraints) {
	// ATM we simply check whether there are nested fluents within the formulae
	
	for (auto condition:goal_conditions) {
		if (condition->nestedness() > 0) return Config::CSPManagerType::Gecode;
	}
	
	for (auto condition:state_constraints) {
		if (condition->nestedness() > 0) return Config::CSPManagerType::Gecode;
	}
	
	return Config::CSPManagerType::Direct;
}

template <typename GecodeHeuristic, typename DirectHeuristic>
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_action_manager_type(const std::vector<GroundAction::cptr>& actions) {
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
