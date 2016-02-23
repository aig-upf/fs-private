

#include <constraints/gecode/gecode_rpg_builder.hxx>
#include <problem.hxx>
#include <utils/utils.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <state.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <languages/fstrips/formulae.hxx>
#include <constraints/gecode/handlers/formula_handler.hxx>



namespace fs0 { namespace gecode {

std::shared_ptr<GecodeRPGBuilder> GecodeRPGBuilder::create(const fs::Formula* goal_formula, const fs::Formula* state_constraints) {
	bool novelty = Config::instance().useNoveltyConstraint();
	bool approximate = Config::instance().useApproximateActionResolution();
	bool dont_care = Config::instance().useElementDontCareOptimization();
	auto conjuncted = goal_formula->conjunction(state_constraints);
	FINFO("main", "Initializing goal CSP Handler with formula:\n" << *conjuncted)
	auto goal_handler = new FormulaCSPHandler(conjuncted, novelty, approximate, dont_care);
	
	// Currently we're make no use of this, but if we do, the state-constraint formula needs to be deep-cloned
// 	auto state_constraint_handler = state_constraints->is_tautology() ? nullptr : new FormulaCSPHandler(state_constraints->clone(), false, novelty, dont_care);
	auto state_constraint_handler = nullptr;
	return std::make_shared<GecodeRPGBuilder>(goal_handler, state_constraint_handler);
}
	

GecodeRPGBuilder::~GecodeRPGBuilder() {
	delete _goal_handler;
	delete _state_constraint_handler;
}
/*
FilteringOutput GecodeRPGBuilder::pruneUsingStateConstraints(GecodeRPGLayer& layer) const {
	if (!_state_constraint_handler) return FilteringOutput::Unpruned;
	
	SimpleCSP* csp = _state_constraint_handler->instantiate_csp(layer);
	bool consistent = csp && csp->checkConsistency();
	
	delete csp;
	
	// TODO This is not entirely correct, as we should be pruning the RPG layers domain. The entire state constraint model needs rethinking, anyway
	assert(0);
	return consistent ? FilteringOutput::Unpruned : FilteringOutput::Failure;
}

*/
//! Don't care about supports, etc.
bool GecodeRPGBuilder::isGoal(const GecodeRPGLayer& layer) const {
	Atom::vctr dummy;
	State dummy_state(0, dummy);
	return isGoal(dummy_state, layer, dummy);
}

bool GecodeRPGBuilder::isGoal(const State& seed, const GecodeRPGLayer& layer, Atom::vctr& support) const {
	assert(support.empty());
	
	bool is_goal = false;
	SimpleCSP* csp = _goal_handler->instantiate_csp(layer);
	
	if (csp && csp->checkConsistency()) {
		FFDEBUG("heuristic", "Formula CSP found to be consistent: " << *csp);
		if (!Config::instance().useApproximateGoalResolution()) {  // Solve the CSP completely
			is_goal = _goal_handler->compute_support(csp, support, seed);
		} else { // Check only local consistency
			is_goal = true;
			_goal_handler->recoverApproximateSupport(csp, support, seed);
		}
	}
	
	delete csp;
	return is_goal;
}

void GecodeRPGBuilder::init_value_selector(const RPGData* bookkeeping) {
	_goal_handler->init(bookkeeping);
// 	_state_constraint_handler->init();
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

