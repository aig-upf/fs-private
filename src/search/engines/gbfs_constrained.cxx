
#include <search/engines/gbfs_constrained.hxx>
#include <problem.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/gecode_crpg.hxx>
#include <heuristics/relaxed_plan/atom_based_crpg.hxx>
#include <heuristics/relaxed_plan/gecode_chmax.hxx>
#include <heuristics/relaxed_plan/direct_crpg.hxx>
#include <heuristics/relaxed_plan/direct_chmax.hxx>
#include <heuristics/relaxed_plan/lifted_crpg.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <constraints/gecode/handlers/ground_action_handler.hxx>
#include <constraints/gecode/handlers/ground_effect_handler.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <actions/applicable_action_set.hxx>
#include <asp/asp_rpg.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace engines {

template <typename GecodeHeuristic, typename DirectHeuristic>
std::unique_ptr<FS0SearchAlgorithm> GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::create(const Config& config, const FS0StateModel& model) const {
	const Problem& problem = model.getTask();
	const std::vector<GroundAction::cptr>& actions = problem.getGroundActions();
	
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	
	FS0SearchAlgorithm* engine = nullptr;
	auto csp_type = decide_csp_type(problem);
	if (csp_type == Config::CSPManagerType::Gecode) {
		FINFO("main", "Chosen CSP Manager: Gecode");
		auto gecode_builder = GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints(), problem.get_tuple_index());
		
		std::vector<std::shared_ptr<BaseActionCSPHandler>> managers;
		if (Config::instance().getCSPModel() == Config::CSPModel::GroundedActionCSP) {
			managers = GroundActionCSPHandler::create(actions, problem.get_tuple_index(), approximate, novelty);
		} else if (Config::instance().getCSPModel() == Config::CSPModel::GroundedEffectCSP) {
			managers = GroundEffectCSPHandler::create(actions, problem.get_tuple_index(), approximate, novelty);
		}  else if (Config::instance().getCSPModel() == Config::CSPModel::ActionSchemaCSP) {
			managers = ActionSchemaCSPHandler::create(problem.getActionSchemata(), problem.get_tuple_index(), approximate, novelty);
		}   else if (Config::instance().getCSPModel() == Config::CSPModel::EffectSchemaCSP) {
			assert(false); // Currently disabled
// 			std::vector<IndexedTupleset> symbol_tuplesets = LiftedCRPG::index_tuplesets(problem.getProblemInfo());
// 			managers = EffectSchemaCSPHandler::create(problem.getActionSchemata(), problem.get_tuple_index(), symbol_tuplesets, approximate, novelty);
		} else {
			throw std::runtime_error("Unknown CSP model type");
		}
		
		
		GecodeHeuristic gecode_builder_heuristic(problem, std::move(managers), std::move(gecode_builder));
		engine = new aptk::StlBestFirstSearch<SearchNode, GecodeHeuristic, FS0StateModel>(model, std::move(gecode_builder_heuristic));
		
	} else {
		auto direct_builder = DirectRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints());
		auto managers = DirectActionManager::create(actions);
		DirectHeuristic direct_builder_heuristic(problem, std::move(managers), std::move(direct_builder));
		
		if (csp_type == Config::CSPManagerType::Direct) {
			FINFO("main", "Chosen CSP Manager: Direct");
			engine = new aptk::StlBestFirstSearch<SearchNode, DirectHeuristic, FS0StateModel>(model, std::move(direct_builder_heuristic));
		} else {
			assert(csp_type == Config::CSPManagerType::ASP);
			FINFO("main", "Chosen CSP Manager: ASP");
			engine = new aptk::StlBestFirstSearch<SearchNode, asp::ASPRPG<DirectHeuristic>, FS0StateModel>(model, asp::ASPRPG<DirectHeuristic>(problem, std::move(direct_builder_heuristic)));
		}
		
	}
	
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
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
Config::CSPManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decide_builder_type(const fs::Formula::cptr goal_formula, const fs::Formula::cptr state_constraints) {
	// ATM we simply check whether there are nested fluents within the formulae
	auto goal_conjunction = dynamic_cast<fs::Conjunction::cptr>(goal_formula);
	// If we have something other than a conjunction, then the gecode manager is required.
	if (!goal_conjunction || (!state_constraints->is_tautology() && !dynamic_cast<fs::Conjunction::cptr>(state_constraints))) return Config::CSPManagerType::Gecode;
	if (goal_formula->nestedness() > 0 || state_constraints->nestedness() > 0) return Config::CSPManagerType::Gecode;
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
template class GBFSConstrainedHeuristicsCreator<ConstrainedRPG, DirectCRPG>;
template class GBFSConstrainedHeuristicsCreator<GecodeCHMax, DirectCHMax>;


} } // namespaces
