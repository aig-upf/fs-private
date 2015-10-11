
#include <heuristics/relaxed_plan/gecode_chmax.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <state_model.hxx>
#include <constraints/direct/direct_rpg_builder.hxx>
#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <constraints/gecode/rpg_layer.hxx>


namespace fs0 { namespace gecode {

GecodeCHMax::GecodeCHMax(const FS0StateModel& model, std::vector<std::shared_ptr<GecodeManager>>&& managers, std::shared_ptr<GecodeRPGBuilder> builder)
	: GecodeCRPG(model, std::move(managers), std::move(builder))
{}
		
long GecodeCHMax::computeHeuristic(const State& seed, const GecodeRPGLayer& state, const RPGData& bookkeeping) {
		if (this->_builder->isGoal(state)) return bookkeeping.getCurrentLayerIdx();
		return -1;
}

} } // namespaces

