
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <languages/fstrips/scopes.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {
	
GecodeFormulaCSPHandler::GecodeFormulaCSPHandler(const std::vector<AtomicFormula::cptr>& conditions)
	:  GecodeCSPHandler(),
	  _conditions(conditions)
{
	createCSPVariables();
	registerFormulaConstraints(_conditions);
	
	Helper::postBranchingStrategy(_base_csp);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = _base_csp.status();
	
	if (st == Gecode::SpaceStatus::SS_SOLVED) {
		FINFO("main", "Formula CSP was statically solved:" << std::endl <<  *this);
	} else if (st == Gecode::SpaceStatus::SS_FAILED) { // This should never happen, as it'd mean that the action is (statically) unapplicable.
		throw std::runtime_error("Formula CSP statically failed");
	} else {
		FINFO("main", "Formula CSP after the initial, static propagation: " << *this);
	}
}


void GecodeFormulaCSPHandler::createCSPVariables() {
	registerFormulaVariables(_conditions);
	if (Config::instance().useNoveltyConstraint()) {
		create_novelty_constraint();
	}
	_translator.perform_registration();
}

std::pair<SimpleCSP*, SimpleCSP*> GecodeFormulaCSPHandler::try_restriction(SimpleCSP* csp, VariableIdx variable, ObjectIdx value) const {
	
		SimpleCSP* restricted = static_cast<SimpleCSP::ptr>(csp->clone());
		
		auto& csp_variable = _translator.resolveInputStateVariable(*restricted, variable);
		Gecode::rel(*restricted, csp_variable, Gecode::IRT_EQ, value);
		Gecode::SpaceStatus st = restricted->status(); // Propagate until fixpoint
		if (st == Gecode::SpaceStatus::SS_FAILED) {
			delete restricted;
			return std::make_pair(nullptr, nullptr);
		}
		
		DFS<SimpleCSP> engine(restricted);
		SimpleCSP* solution = engine.next();
		if (!solution) {
			delete restricted;
			return std::make_pair(nullptr, nullptr);
		}
		
		return std::make_pair(restricted, solution);
}

SimpleCSP* GecodeFormulaCSPHandler::constrain_solutions(SimpleCSP* base, const State& seed, std::vector<VariableIdx>& unprocessed) const {
	assert(unprocessed.empty());

	SimpleCSP* current = base;
	DFS<SimpleCSP> engine(current);
	SimpleCSP* most_constrained = engine.next();
	if (!most_constrained) return nullptr;
	
	if (!Config::instance().useEnhancedGoalResolution()) {
		for (const auto& it:_translator.getAllInputVariables()) unprocessed.push_back(it.first);
		return most_constrained;
	}
	
	for (const auto& it:_translator.getAllInputVariables()) {
		VariableIdx variable = it.first;
		
		auto csps = try_restriction(current, variable, seed.getValue(variable));
		if (!csps.first) {
			unprocessed.push_back(variable);
			continue;
		}
		
		// Otherwise, we were able to constraint the value of a variable to its seed value and still have a solution,
		// now we keep trying forward with other variables
		if (current != base) delete current;
		current = csps.first;
		delete most_constrained;
		most_constrained = csps.second;
	}
	
	if (current != base) delete current;
	
	return most_constrained;
}

bool GecodeFormulaCSPHandler::compute_support(SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	std::vector<VariableIdx> unprocessed;
	SimpleCSP* solution = constrain_solutions(csp, seed, unprocessed);
	if (!solution) return false;
	
	FFDEBUG("heuristic", "Most constrained formula solution found: " << *solution);
	
	// First process the direct state variables
	for (VariableIdx variable:unprocessed) {
		support.push_back(Atom(variable, _translator.resolveInputStateVariableValue(*solution, variable)));
	}
	
	// Now process the indirect state variables
	std::set<VariableIdx> inserted;
	for (auto fluent:formula_nested_fluents) {
		VariableIdx variable = _translator.resolveNestedFluent(fluent).resolveStateVariable(*solution);
		
		if (inserted.find(variable) == inserted.end()) { // Don't push twice to the support the same atom
			ObjectIdx value = _translator.resolveValue(fluent, CSPVariableType::Input, *solution);
			support.push_back(Atom(variable, value));
			inserted.insert(variable);
		}
	}
	
	delete solution;
	return true;
}

bool GecodeFormulaCSPHandler::check_solution_exists(SimpleCSP* csp) const {
	DFS<SimpleCSP> engine(csp);
	SimpleCSP* solution = engine.next();
	if (!solution) return false;
	delete solution;
	return true;
}

void GecodeFormulaCSPHandler::recoverApproximateSupport(gecode::SimpleCSP* csp, Atom::vctr& support, const State& seed) const {
	// We have already propagated constraints with the call to status(), so we simply arbitrarily pick one consistent value per variable.
	
	// First process the direct state variables
	for (const auto& it:_translator.getAllInputVariables()) {
		VariableIdx planning_variable = it.first;
		const Gecode::IntVar& csp_var = csp->_intvars[it.second];
		IntVarValues values(csp_var);  // This returns a set with all consistent values for the given variable
		assert(values()); // Otherwise the CSP would be inconsistent!
		
		// If the original value makes the situation a goal, then we don't need to add anything for this variable.
		int seed_value = seed.getValue(planning_variable);
		int selected = Helper::selectValueIfExists(values, seed_value);
		if (selected == seed_value) continue;
		support.push_back(Atom(planning_variable, selected)); // It not, we simply pick the first consistent value
	}
	
	// Now process the indirect state variables
	// TODO - This part on approximate recovery of values for nested term fluents is a bit iffy.
	// For state variables acting as nested fluent indexes, we are not ensuring that the same value is selected for them
	// when acting as support and when acting as index - but this is subject to change when we move into arity > 1 nested fluents,
	// so I don't think it is worth the extra effort refactoring this now.
	std::set<VariableIdx> inserted;
	for (auto fluent:formula_nested_fluents) {
		auto nested_translator = _translator.resolveNestedFluent(fluent);
		auto idx_variable = nested_translator.getIndex(*csp);
		IntVarValues values(idx_variable);
		assert(values()); // Otherwise the CSP would be inconsistent!
		int arbitrary_element = values.val();
		nested_translator.getTableVariables()[arbitrary_element];
		VariableIdx state_variable = nested_translator.getTableVariables()[arbitrary_element];
		
		if (inserted.find(state_variable) == inserted.end()) { // Don't push twice to the support the same atom
			auto csp_var = _translator.resolveVariable(fluent, CSPVariableType::Input, *csp);
			IntVarValues values(csp_var);
			assert(values()); // Otherwise the CSP would be inconsistent!
			support.push_back(Atom(state_variable, values.val())); // Simply push an arbitrary value
			inserted.insert(state_variable);
		}
	}
}

void GecodeFormulaCSPHandler::index_scopes() {
	ScopeUtils::TermSet nested;
	for (unsigned i = 0; i < _conditions.size(); ++i) {
		ScopeUtils::computeIndirectScope(_conditions[i], nested);
	}
	formula_nested_fluents = std::vector<fs::FluentHeadedNestedTerm::cptr>(nested.cbegin(), nested.cend());
}

void GecodeFormulaCSPHandler::create_novelty_constraint() {
	// First we collect both the sets of state variables and derived variables which are present in the RHS of the effects.
	std::set<VariableIdx> direct;
	std::set<VariableIdx> derived;
	
	for (auto condition:_conditions) {
		ScopeUtils::computeVariables(condition, direct, derived);
	}
	
	// Now we register the adequate variables through the NoveltyConstraint object
	_novelty = WeakNoveltyConstraint::create(_translator, _conditions, {});
}


} } // namespaces
