
#include <heuristics/relaxed_plan/hmax.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <relaxed_state.hxx>
#include <state_model.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


namespace fs0 {
	
template <typename RPGBuilder>
HMaxHeuristic<RPGBuilder>::HMaxHeuristic(const FS0StateModel& model, std::vector<std::shared_ptr<BaseActionManager>>&& managers, std::shared_ptr<RPGBuilder> builder)
	: ConstrainedRelaxedPlanHeuristic<RPGBuilder>(model, std::move(managers), builder)
{}
		
template <typename RPGBuilder>
float HMaxHeuristic<RPGBuilder>::computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg) {
		if (this->_builder->isGoal(state)) return rpg.getCurrentLayerIdx();
		return -1;
}

// explicit instantiations
template class HMaxHeuristic<fs0::DirectRPGBuilder>;
template class HMaxHeuristic<fs0::gecode::GecodeRPGBuilder>;

} // namespaces

