
#include <search/engines/gbfs_constrained.hxx>
#include <problem.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <heuristics/relaxed_plan/constrained_relaxed_plan_heuristic.hxx>
#include <heuristics/relaxed_plan/constrained_hmax.hxx>
#include <heuristics/relaxed_plan/action_managers/action_manager_factory.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace engines {

template <typename GecodeHeuristic, typename DirectHeuristic>
std::unique_ptr<FS0SearchAlgorithm> GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::create(const Config& config, const FS0StateModel& model) const {
	const Problem& problem = model.getTask();
	auto action_managers = ActionManagerFactory::create(problem.getGroundActions());
	
	FS0SearchAlgorithm* engine = nullptr;
	if (decideRPGBuilderType(problem) == Config::GoalManagerType::Gecode) {
		std::shared_ptr<GecodeRPGBuilder> gecode_builder = std::shared_ptr<GecodeRPGBuilder>(GecodeRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints()));
		GecodeHeuristic gecode_builder_heuristic(model, std::move(action_managers), gecode_builder);
		engine = new aptk::StlBestFirstSearch<SearchNode, GecodeHeuristic, FS0StateModel>(model, std::move(gecode_builder_heuristic));
		
	} else {
		std::shared_ptr<DirectRPGBuilder> direct_builder = std::shared_ptr<DirectRPGBuilder>(DirectRPGBuilder::create(problem.getGoalConditions(), problem.getStateConstraints()));
		DirectHeuristic direct_builder_heuristic(model, std::move(action_managers), direct_builder);
		engine = new aptk::StlBestFirstSearch<SearchNode, DirectHeuristic, FS0StateModel>(model, std::move(direct_builder_heuristic));
	}
	
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

template <typename GecodeHeuristic, typename DirectHeuristic>
bool GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::needsGecodeRPGBuilder(const std::vector<fs::AtomicFormula::cptr>& goal_conditions, const std::vector<fs::AtomicFormula::cptr>& state_constraints) {
	// ATM we simply check whether there are nested fluents within the formulae
	
	for (auto condition:goal_conditions) {
		if (condition->nestedness() > 0) return true;
	}
	
	for (auto condition:state_constraints) {
		if (condition->nestedness() > 0) return true;
	}
	
	return false;
}

template <typename GecodeHeuristic, typename DirectHeuristic>
Config::GoalManagerType GBFSConstrainedHeuristicsCreator<GecodeHeuristic, DirectHeuristic>::decideRPGBuilderType(const Problem& problem) {
	if (Config::instance().getGoalManagerType() == Config::GoalManagerType::Gecode || needsGecodeRPGBuilder(problem.getGoalConditions(), problem.getStateConstraints())) {
		FINFO("main", "Chosen RPG Builder: Gecode");
		return Config::GoalManagerType::Gecode;
	} else {
		FINFO("main", "Chosen RPG Builder: Direct");
		return Config::GoalManagerType::Basic;
	}
}

// explicit instantiations
template class GBFSConstrainedHeuristicsCreator<ConstrainedRelaxedPlanHeuristic<GecodeRPGBuilder>, ConstrainedRelaxedPlanHeuristic<DirectRPGBuilder>>;
template class GBFSConstrainedHeuristicsCreator<ConstrainedHMaxHeuristic<GecodeRPGBuilder>, ConstrainedHMaxHeuristic<DirectRPGBuilder>>;


} } // namespaces