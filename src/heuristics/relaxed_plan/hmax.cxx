
#include <heuristics/relaxed_plan/hmax.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <relaxed_state.hxx>
#include <state_model.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>


namespace fs0 {

template <typename Model, typename RPGBuilder>
float HMaxHeuristic<Model, RPGBuilder>::computeHeuristic(const State& seed, const RelaxedState& state, const RPGData& rpg) {
		if (this->_builder->isGoal(state)) return rpg.getCurrentLayerIdx();
		return -1;
}

// explicit instantiations
template class HMaxHeuristic<fs0::FS0StateModel, fs0::DirectRPGBuilder>;
template class HMaxHeuristic<fs0::FS0StateModel, fs0::gecode::GecodeRPGBuilder>;

} // namespaces

