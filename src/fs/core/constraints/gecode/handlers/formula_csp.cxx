
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/constraints/gecode/handlers/formula_csp.hxx>
#include <fs/core/constraints/gecode/helper.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/printers/gecode.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/constraints/gecode/utils/novelty_constraints.hxx>
#include <fs/core/constraints/gecode/supports.hxx>
#include <fs/core/state.hxx>

#include <gecode/driver.hh>
#include <fs/core/constraints/gecode/utils/value_selection.hxx>

namespace fs0 { namespace gecode {
	
FormulaCSP::FormulaCSP(const fs::Formula* formula, const AtomIndex& tuple_index, bool approximate)
	:  BaseCSP(tuple_index, approximate),
	  _formula(formula)
{
	LPT_DEBUG("translation", "Gecode Formula Handler: processing goal formula " << *_formula);
	index();
	
	createCSPVariables(false);
	register_csp_constraints();
	index_existential_variable_uses();
	
	Helper::postBranchingStrategy(*_base_csp);
	
//	 std::cout << "Goal CSP:" << std::endl << _translator << std::endl;
	
	// MRJ: in order to be able to clone a CSP, we need to ensure that it is "stable" i.e. propagate all constraints until fixed point
	Gecode::SpaceStatus st = _base_csp->status();
	
	if (st == Gecode::SpaceStatus::SS_SOLVED) {
		LPT_INFO("main", "Formula CSP was statically solved:" << std::endl <<  *this);
	} else if (st == Gecode::SpaceStatus::SS_FAILED) {
		LPT_INFO("main", "Formula CSP statically failed:" << *this);
		_failed = true;
	} else {
		LPT_INFO("main", "Formula CSP after the initial, static propagation: " << *this);
	}
	
	index_scopes(); // This needs to be _after_ the CSP variable registration
}

FormulaCSP::~FormulaCSP() { delete _formula; }

bool FormulaCSP::compute_support(GecodeSpace* csp, std::vector<AtomIdx>& support) const {
	GecodeSpace* solution = compute_single_solution(csp);
	if (!solution) return false;
	
	LPT_EDEBUG("heuristic", "Formula CSP solution found: " << fs0::print::csp(_translator, *solution));
	assert(support.empty());
	support = Supports::extract_support(solution, _translator, _tuple_indexes, _necessary_tuples);
	delete solution;
	LPT_EDEBUG("heuristic", "Support for the formula is:" << std::endl << fs0::print::support(support));
	return true;
}

bool FormulaCSP::is_satisfiable(GecodeSpace* csp) const {
	GecodeSpace* solution = compute_single_solution(csp);
	if (!solution) return false;
	delete solution;
	return true;
}

GecodeSpace* FormulaCSP::compute_single_solution(GecodeSpace* csp) {
	Gecode::DFS<GecodeSpace> engine(csp);
	return engine.next();
}

void FormulaCSP::index_scopes() {
	// Register all fluent symbols involved
	_tuple_indexes = _translator.index_fluents(_all_terms);
}

// In the case of a single formula, we just retrieve and index all terms and atoms
void FormulaCSP::index() {
	// Index formula elements
	index_csp_elements({_formula});
}

void FormulaCSP::init_value_selector(const RPGIndex* graph) {
	_base_csp->init_value_selector(std::make_shared<TupleMinHMaxValueSelector>(&_tuple_index, &_translator, graph));
}

void FormulaCSP::index_existential_variable_uses() {
	const ProblemInfo& info = ProblemInfo::getInstance();
	
	// This is part of a very preliminary implementation of a goal support selection mechanism that 
	// selects for existential variables those values that induce goal atoms with minimum sums of hmax values.
	// That is, if an existential variable Z participates in 2 goal atoms p(Z) and q (Z, c), we will prioritize
	// the possible values z of Z such that hmax(p(z)) + hmax(q(z,c)) is minimum, i.e. such that the induced
	// atoms are achieved the earliest possible

	std::vector<std::vector<std::unordered_map<int, AtomIdx>>> existential_data;
	existential_data.resize(_base_csp->_intvars.size());
	
	for (const ExtensionalConstraint& extensional:_extensional_constraints) {
		const fs::FluentHeadedNestedTerm* fluent = extensional.get_term();
		if (!info.isPredicate(fluent->getSymbolId())) continue; // TODO We should instead consider functional fluents as well
		
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
					subterm_values.push_back(make_object<int>(0));
					continue;
				}
				
				if (const auto* constant = dynamic_cast<const fs::Constant*>(term)) {
					subterm_values.push_back(constant->getValue());
				} else {
					rest_subterms_are_constant = false;
					break;
				}
			}
			
			if (!rest_subterms_are_constant) continue;
			assert(ex_var_position != -1);
			
			std::unordered_map<int, AtomIdx> variable_resolutions;
			
			for (const object_id& value:info.getTypeObjects(variable->getType())) {
				subterm_values.at(ex_var_position) = value;
				AtomIdx tuple_id = _tuple_index.to_index(fluent->getSymbolId(), subterm_values);
				variable_resolutions.insert(std::make_pair(fs0::value<int>(value), tuple_id));
			}
			
			unsigned variable_idx = _translator.resolveVariableIndex(variable);
			existential_data.at(variable_idx).push_back(variable_resolutions);
		}
	}
	
	_translator.set_existential_data(std::move(existential_data));
}

} } // namespaces
