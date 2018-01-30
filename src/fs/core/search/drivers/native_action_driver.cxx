
#include <fs/core/search/drivers/native_action_driver.hxx>

#include <fs/core/search/utils.hxx>

#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/utils/support.hxx>

#include <fs/core/constraints/native/action_handler.hxx>


using namespace fs0::gecode;

namespace fs0 { namespace drivers {

template <>
gecode::NativeRPG*
NativeActionDriver<gecode::NativeRPG>::configure_heuristic(const Problem& problem, const Config& config) {

	const auto& tuple_index = problem.get_tuple_index();
    const auto& actions = problem.getGroundActions();
	auto managers = NativeActionHandler::create(actions, tuple_index, true);


    const auto managed = support::compute_managed_symbols(std::vector<const ActionBase*>(actions.begin(), actions.end()), problem.getGoalConditions(), {});
    ExtensionHandler extension_handler(problem.get_tuple_index(), managed);

    auto heuristic = new gecode::NativeRPG(problem, problem.getGoalConditions(), {}, std::move(managers), extension_handler);
//    auto heuristic = new HeuristicT(problem);
    return heuristic;
}

template <>
UnsatisfiedGoalAtomsCounter*
NativeActionDriver<UnsatisfiedGoalAtomsCounter>::configure_heuristic(const Problem& problem, const Config& config) {
    return new UnsatisfiedGoalAtomsCounter(problem);
}


template <typename HeuristicT>
typename NativeActionDriver<HeuristicT>::EnginePT
NativeActionDriver<HeuristicT>::create(const Config& config, const GroundStateModel& model, SearchStats& stats) {
	LPT_INFO("main", "Using the native action driver");
	const Problem& problem = model.getTask();

	_heuristic = std::unique_ptr<HeuristicT>(configure_heuristic(problem, config));

	EventUtils::setup_stats_observer<NodeT>(stats, _handlers);
	EventUtils::setup_evaluation_observer<NodeT, HeuristicT>(config, *_heuristic, stats, _handlers);

	auto engine = new EngineT(model, gecode::build_monotonicity_csp(problem, config));
	lapkt::events::subscribe(*engine, _handlers);

	return EnginePT(engine);
}
template <typename HeuristicT>
GroundStateModel
NativeActionDriver<HeuristicT>::setup(Problem& problem) {
	return GroundingSetup::fully_ground_model(problem);
}

template <typename HeuristicT>
ExitCode
NativeActionDriver<HeuristicT>::search(Problem& problem, const Config& config, const EngineOptions& options, float start_time) {
	GroundStateModel model = setup(problem);
	SearchStats stats;
	bool actionless = model.getTask().getGroundActions().empty();
	auto engine = create(config, model, stats);
	return Utils::SearchExecution<GroundStateModel>(model).do_search(*engine, options, start_time, stats, actionless);

}


template class NativeActionDriver<gecode::NativeRPG>;
template class NativeActionDriver<UnsatisfiedGoalAtomsCounter>;

} } // namespaces
