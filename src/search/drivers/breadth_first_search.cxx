
#include <search/drivers/breadth_first_search.hxx>
#include <search/events.hxx>
#include <actions/ground_action_iterator.hxx>
#include <actions/grounding.hxx>
#include <problem_info.hxx>
#include <problem.hxx>

namespace fs0 { namespace drivers {
	
BreadthFirstSearchDriver::Engine
BreadthFirstSearchDriver::create(const Config& config, const GroundStateModel& model) {
	
	using StatsT = StatsObserver<NodeT>;
	_handlers.push_back(std::unique_ptr<StatsT>(new StatsT(_stats)));
	
	auto engine = new lapkt::StlBreadthFirstSearch<NodeT, GroundStateModel>(model);
	lapkt::events::subscribe(*engine, _handlers);
	
	return Engine(engine);
}

GroundStateModel
BreadthFirstSearchDriver::setup(const Config& config, Problem& problem) const {
	problem.setGroundActions(ActionGrounder::fully_ground(problem.getActionData(), ProblemInfo::getInstance()));
	return GroundStateModel(problem); // By default we ground all actions and return a model with the problem as it is
}

} } // namespaces
