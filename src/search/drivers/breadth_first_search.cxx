
#include <search/drivers/breadth_first_search.hxx>
#include <search/events.hxx>
#include <search/utils.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <problem.hxx>
#include <search/drivers/setups.hxx>


namespace fs0 { namespace drivers {
	
BreadthFirstSearchDriver::Engine
BreadthFirstSearchDriver::create(const Config& config, const GroundStateModel& model) {
	
	EventUtils::setup_stats_observer<NodeT>(_stats, _handlers);
	auto engine = new lapkt::StlBreadthFirstSearch<NodeT, GroundStateModel>(model);
	lapkt::events::subscribe(*engine, _handlers);
	
	return Engine(engine);
}

GroundStateModel
BreadthFirstSearchDriver::setup(Problem& problem) const {
	return GroundingSetup::fully_ground_model(problem);
}

void 
BreadthFirstSearchDriver::search(Problem& problem, const Config& config, const std::string& out_dir, float start_time) {
	GroundStateModel model = setup(problem);
	auto engine = create(config, model);
	Utils::do_search(*engine, model, out_dir, start_time, _stats);
}

} } // namespaces
