

#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <state.hxx>
#include <heuristics/relaxed_plan/gecode_rpg_layer.hxx>


namespace fs0 { namespace gecode {

GecodeRPGBuilder::cptr GecodeRPGBuilder::create(const std::vector<AtomicFormula::cptr>& goal_conditions, const std::vector<AtomicFormula::cptr>& state_constraints) {
	auto goal_handler = new GecodeFormulaCSPHandler(Utils::merge(goal_conditions, state_constraints));
	auto state_constraint_handler = state_constraints.empty() ? nullptr : new GecodeFormulaCSPHandler(state_constraints);
	return new GecodeRPGBuilder(goal_handler, state_constraint_handler);
}
	

GecodeRPGBuilder::~GecodeRPGBuilder() {
	delete _goal_handler;
	delete _state_constraint_handler;
}
/*
FilteringOutput GecodeRPGBuilder::pruneUsingStateConstraints(GecodeRPGLayer& layer) const {
	if (!_state_constraint_handler) return FilteringOutput::Unpruned;
	
	SimpleCSP* csp = _state_constraint_handler->instantiate_csp(layer);
	bool consistent = csp->checkConsistency();
	
	delete csp;
	
	// TODO This is not entirely correct, as we should be pruning the RPG layers domain. The entire state constraint model needs rethinking, anyway
	assert(0);
	return consistent ? FilteringOutput::Unpruned : FilteringOutput::Failure;
}

*/
//! Don't care about supports, etc.
bool GecodeRPGBuilder::isGoal(const RelaxedState& layer, const GecodeRPGLayer& gecode_layer, const GecodeRPGLayer& delta_layer) const {
	Atom::vctr dummy;
	State dummy_state(0 ,dummy);
	return isGoal(dummy_state, layer, gecode_layer, delta_layer, dummy);
}

bool GecodeRPGBuilder::isGoal(const State& seed, const RelaxedState& layer, const GecodeRPGLayer& gecode_layer, const GecodeRPGLayer& delta_layer, Atom::vctr& support) const {
	assert(support.empty());
	
	FDEBUG("heuristic", "Checking goal with Gecode Layer: " << gecode_layer);
	FDEBUG("heuristic", "Checking goal with Delta Layer: " << delta_layer);
	
	SimpleCSP* csp = _goal_handler->instantiate_csp(gecode_layer, delta_layer);
	bool is_goal = false;
	
	if (csp->checkConsistency()) {
		FFDEBUG("heuristic", "Formula CSP found to be consistent: " << *csp);
		if (Config::instance().getGoalResolutionType() == Config::CSPResolutionType::Full) {  // Solve the CSP completely
			is_goal = _goal_handler->compute_support(csp, support, seed);
		} else { // Check only local consistency
			is_goal = true;
			_goal_handler->recoverApproximateSupport(csp, support, seed);
		}
	}
	
	delete csp;
	return is_goal;
}

std::ostream& GecodeRPGBuilder::print(std::ostream& os) const {
	os  << std::endl << "GecodeRPGBuilder" << std::endl;
	os << "----------------" << std::endl;
	os << "Goal handler: " << std::endl << *_goal_handler << std::endl;
	if (_state_constraint_handler) os << "State constraint handler: " << std::endl << *_state_constraint_handler << std::endl;
	os  << std::endl;
	return os;
}



} } // namespaces

