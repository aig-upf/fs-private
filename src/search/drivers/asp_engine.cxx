
#include <search/drivers/asp_engine.hxx>
#include <search/nodes/astar_search_node.hxx>
#include <search/nodes/heuristic_search_node.hxx>
#include <utils/config.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <actions/ground_action_iterator.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <asp/asp_hplus.hxx>

namespace fs0 { namespace drivers {
	
std::unique_ptr<FS0SearchAlgorithm> ASPEngine::create(const Config& config, const GroundStateModel& model) const {
	const Problem& problem = model.getTask();
	
	validate(problem);
	
	FS0SearchAlgorithm* engine = nullptr;
	std::string search = config.getOption<std::string>("engine.search");
	if (search == "astar") {
		LPT_INFO("main", "Chosen engine: A* with ASP-based h+ computation");
		typedef AStarSearchNode<State, GroundAction> AStarNode;
		engine = new aptk::StlBestFirstSearch<AStarNode, asp::ASPHPlus, GroundStateModel>(model, asp::ASPHPlus(problem));
	} else {
		assert(search == "gbfs");
		LPT_INFO("main", "Chosen engine: GBFS with ASP-based h+ computation");
		typedef HeuristicSearchNode<State, GroundAction> GBFSNode;
		engine = new aptk::StlBestFirstSearch<GBFSNode, asp::ASPHPlus, GroundStateModel>(model, asp::ASPHPlus(problem));
	}
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

void ASPEngine::validate(const Problem& problem) {
	// ATM we don't perform any validation
}


} } // namespaces
