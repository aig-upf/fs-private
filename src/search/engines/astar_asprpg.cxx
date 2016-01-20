
#include <search/engines/astar_asprpg.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <actions/applicable_action_set.hxx>
#include <state_model.hxx>
#include <utils/config.hxx>
#include <utils/logging.hxx>
#include <aptk2/search/algorithms/best_first_search.hxx>
#include <asp/asp_hplus.hxx>

namespace fs0 { namespace engines {
	
std::unique_ptr<FS0SearchAlgorithm> AStarASPRPG::create(const Config& config, const FS0StateModel& model) const {
	typedef AStarSearchNode<State, GroundAction> SearchNode;
	
	const Problem& problem = model.getTask();
	FINFO("main", "Chosen engine: A* with ASP-based h+ computation");
	
	validate(problem);
	
	FS0SearchAlgorithm* engine = new aptk::StlBestFirstSearch<SearchNode, asp::ASPHPlus, FS0StateModel>(model, asp::ASPHPlus(problem));
	return std::unique_ptr<FS0SearchAlgorithm>(engine);
}

void AStarASPRPG::validate(const Problem& problem) {
	// ATM we don't perform any validation
}


} } // namespaces
