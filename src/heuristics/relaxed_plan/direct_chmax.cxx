
#include <heuristics/relaxed_plan/direct_chmax.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <relaxed_state.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


namespace fs0 {

DirectCHMax::DirectCHMax(const Problem& problem, std::vector<std::shared_ptr<DirectActionManager>>&& managers, std::shared_ptr<DirectRPGBuilder> builder)
	: DirectCRPG(problem, std::move(managers), std::move(builder))
{}

long DirectCHMax::computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& bookkeeping) {
		if (this->_builder->isGoal(state)) return bookkeeping.getCurrentLayerIdx();
		return -1;
}

} // namespaces

