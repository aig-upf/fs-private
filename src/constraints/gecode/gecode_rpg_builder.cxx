

#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <state.hxx>


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

FilteringOutput GecodeRPGBuilder::pruneUsingStateConstraints(RelaxedState& state) const {
	if (!_state_constraint_handler) return FilteringOutput::Unpruned;
	
	// TODO TODO TODO
	assert(0);
	
	return FilteringOutput::Unpruned;
}


//! Don't care about supports, etc.
bool GecodeRPGBuilder::isGoal(const RelaxedState& layer) const {
	Atom::vctr dummy;
	State dummy_state(0 ,dummy);
	return isGoal(dummy_state, layer, dummy);
}

bool GecodeRPGBuilder::isGoal(const State& seed, const RelaxedState& layer, Atom::vctr& support) const {
	assert(support.empty());
	SimpleCSP* csp = _goal_handler->instantiate_csp(layer);
	bool is_goal = false;
	
	if (csp->checkConsistency()) {
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




} } // namespaces

