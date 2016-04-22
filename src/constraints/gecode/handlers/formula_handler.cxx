
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <constraints/gecode/handlers/formula_handler.hxx>
#include <constraints/gecode/helper.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <utils/logging.hxx>
#include <utils/tuple_index.hxx>
#include <utils/utils.hxx>
#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <constraints/gecode/supports.hxx>
#include <state.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {
	
FormulaHandler::FormulaHandler(const fs::Formula* formula, const TupleIndex& tuple_index, bool approximate)
	:  BaseCSPHandler(tuple_index, approximate),
	  _formula(formula)
{
	setup();
	
	createCSPVariables(false);
	register_csp_constraints();
	index_existential_variable_uses();
	
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

FormulaHandler::~FormulaHandler() { delete _formula; }

bool FormulaHandler::compute_support(SimpleCSP* csp, std::vector<TupleIdx>& support) const {
	SimpleCSP* solution = compute_single_solution(csp);
	if (!solution) return false;
	
	FFDEBUG("heuristic", "Formula CSP solution found: " << *solution);
	assert(support.empty());
	support = Supports::extract_support(solution, _translator, _tuple_indexes, _necessary_tuples);
	delete solution;
	return true;
}

bool FormulaHandler::is_satisfiable(SimpleCSP* csp) const {
	SimpleCSP* solution = compute_single_solution(csp);
	if (!solution) return false;
	delete solution;
	return true;
}

SimpleCSP* FormulaHandler::compute_single_solution(SimpleCSP* csp) {
	Gecode::DFS<SimpleCSP> engine(csp);
	return engine.next();
}

void FormulaHandler::index_scopes() {
	// Register all fluent symbols involved
	_tuple_indexes = _translator.index_fluents(_all_terms);
}

// In the case of a single formula, we just retrieve and index all terms and atoms
void FormulaHandler::index() {
	const auto conditions =  _formula->all_atoms();
	const auto terms = _formula->all_terms();
	
	// Index formula elements
	index_formula_elements(conditions, terms);
}

void FormulaHandler::init_value_selector(const RPGIndex* graph) {
	_base_csp.init_value_selector(std::make_shared<TupleMinHMaxValueSelector>(&_tuple_index, &_translator, graph));
}

void FormulaHandler::index_existential_variable_uses() {
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	// This is part of a very preliminary implementation of a goal support selection mechanism that 
	// selects for existential variables those values that induce goal atoms with minimum sums of hmax values.
	// That is, if an existential variable Z participates in 2 goal atoms p(Z) and q (Z, c), we will prioritize
	// the possible values z of Z such that hmax(p(z)) + hmax(q(z,c)) is minimum, i.e. such that the induced
	// atoms are achieved the earliest possible

	std::vector<std::vector<std::unordered_map<int, TupleIdx>>> existential_data;
	existential_data.resize(_base_csp._intvars.size());
	
	for (const ExtensionalConstraint& extensional: _extensional_constraints) {
		const fs::FluentHeadedNestedTerm* fluent = extensional.get_term();
		
		auto variables = Utils::filter_by_type<const fs::BoundVariable*>(fluent->getSubterms());
		for (const fs::BoundVariable* variable:variables) {
			
			bool rest_subterms_are_constant = true;
			int ex_var_position = -1;
			ValueTuple subterm_values;
			for (unsigned i = 0; i < fluent->getSubterms().size(); ++i) {
				const fs::Term* term = fluent->getSubterms().at(i);
				if (*term == *variable) {
					assert(ex_var_position == -1);
					ex_var_position = i;
					subterm_values.push_back(0);
					continue;
				}
				
				if (const fs::Constant* constant = dynamic_cast<const fs::Constant*>(term)) {
					subterm_values.push_back(constant->getValue());
				} else {
					rest_subterms_are_constant = false;
					break;
				}
			}
			
			if (!rest_subterms_are_constant) continue;
			assert(ex_var_position != -1);
			
			std::unordered_map<int, TupleIdx> variable_resolutions;
			
			for (ObjectIdx value:info.getTypeObjects(variable->getType())) {
				subterm_values.at(ex_var_position) = value;
				TupleIdx tuple_id = _tuple_index.to_index(fluent->getSymbolId(), subterm_values);
				variable_resolutions.insert(std::make_pair(value, tuple_id));
			}
			
			unsigned variable_idx = _translator.resolveVariableIndex(variable);
			existential_data.at(variable_idx).push_back(variable_resolutions);
		}
	}
	
	_translator.set_existential_data(std::move(existential_data));
}

} } // namespaces
