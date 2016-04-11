
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/scopes.hxx>
#include <constraints/gecode/handlers/lifted_formula_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/logging.hxx>
#include <utils/config.hxx>
#include <utils/tuple_index.hxx>
#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <constraints/gecode/supports.hxx>
#include <state.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {
	
LiftedFormulaHandler::LiftedFormulaHandler(const fs::Formula::cptr formula, const TupleIndex& tuple_index, bool approximate)
	:  BaseCSPHandler(tuple_index, approximate),
	  _formula(formula)
{
	setup();
	
	createCSPVariables(false);
	register_csp_constraints();
	
	Helper::postBranchingStrategy(_base_csp);
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = _base_csp.status();
	
	if (st == Gecode::SpaceStatus::SS_SOLVED) {
		FINFO("main", "Formula CSP was statically solved:" << std::endl <<  *this);
	} else if (st == Gecode::SpaceStatus::SS_FAILED) {
		FINFO("main", "Formula CSP statically failed:" << *this);
		_failed = true;
	} else {
		FINFO("main", "Formula CSP after the initial, static propagation: " << *this);
	}
	
	index_scopes(); // This needs to be _after_ the CSP variable registration
}

LiftedFormulaHandler::~LiftedFormulaHandler() { delete _formula; }

bool LiftedFormulaHandler::compute_support(SimpleCSP* csp, std::vector<TupleIdx>& support, const State& seed) const {
	Gecode::DFS<SimpleCSP> engine(csp);
	SimpleCSP* solution = engine.next();
	if (!solution) return false;
	
	FFDEBUG("heuristic", "Formula CSP solution found: " << *solution);
	assert(support.empty());
	support = Supports::extract_support(solution, _translator, _tuple_indexes);
	
	delete solution;
	return true;
}


void LiftedFormulaHandler::index_scopes() {
	// Register all fluent symbols involved
	_tuple_indexes = _translator.index_fluents(_all_terms);
}

bool LiftedFormulaHandler::check_solution_exists(SimpleCSP* csp) const {
	Gecode::DFS<SimpleCSP> engine(csp);
	SimpleCSP* solution = engine.next();
	if (!solution) return false;
// 	std::cout << "Formula solution: " << std::endl; print(std::cout, *solution); std::cout << std::endl;
	delete solution;
	return true;
}


// In the case of a single formula, we just retrieve and index all terms and atoms
void LiftedFormulaHandler::index() {
	const auto conditions =  _formula->all_atoms();
	const auto terms = _formula->all_terms();
	
	// Index formula elements
	index_formula_elements(conditions, terms);
}

// TODO CODE DUPLICATION WITH EffectSchemaCSPHandler
SimpleCSP::ptr LiftedFormulaHandler::instantiate(const RPGIndex& rpg) const {
	if (_failed) return nullptr;
	SimpleCSP* clone = static_cast<SimpleCSP::ptr>(_base_csp.clone());
	_translator.updateStateVariableDomains(*clone, rpg.get_domains());
	for (const ExtensionalConstraint& constraint:_extensional_constraints) {
		if (!constraint.update(*clone, _translator, rpg)) {
			delete clone;
			return nullptr;
		}
	}
	return clone;
}

void LiftedFormulaHandler::init_value_selector(const RPGIndex* graph) {
	_base_csp.init_value_selector(std::make_shared<TupleMinHMaxValueSelector>(&_tuple_index, &_translator, graph));
}


} } // namespaces
