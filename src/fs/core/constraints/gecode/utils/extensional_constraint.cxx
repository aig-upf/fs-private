
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/constraints/gecode/utils/extensional_constraint.hxx>
#include <fs/core/state.hxx>
#include <fs/core/constraints/gecode/gecode_csp.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>
#include <fs/core/constraints/gecode/extensions.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/utils/printers/gecode.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>

namespace fs0 { namespace gecode {

ExtensionalConstraint::ExtensionalConstraint(const fs::FluentHeadedNestedTerm* term, const AtomIndex& tuple_index, bool predicate)
	: _predicate(predicate), _term(term), _variable_idx(-1), _tuple_index(tuple_index)
{}

void ExtensionalConstraint::register_constraints(CSPTranslator& translator) {
	LPT_DEBUG("translation", "Preprocessing extensional constraints for term " << *_term << ", predicate?: " << _predicate);
	const ProblemInfo& info = ProblemInfo::getInstance();

	for (auto subterm:_term->getSubterms()) {
		_subterm_variable_indexes.push_back(translator.resolveVariableIndex(subterm));
	}

	if (!_predicate) _term_variable_index = translator.resolveVariableIndex(_term);

	if (_term->getSubterms().empty()) {
		_variable_idx = info.resolveStateVariable(_term->getSymbolId(), {});
	}
}

// Updating the CSP from a non-relaxed state is necessary to efficiently implement satisfiability tests on states
// with formulas involving existential variables
bool ExtensionalConstraint::update(GecodeCSP& csp, const CSPTranslator& translator, const State& state) const {
	if (_variable_idx >= 0) { // If the predicate is 0-ary, there is no actual extension, we thus treat the case specially.
		return int(state.getValue(_variable_idx)) == 1;
	} else {
		return update(csp, translator, compute_extension(_term->getSymbolId(), state));
	}
}

bool ExtensionalConstraint::update(GecodeCSP& csp, const CSPTranslator& translator, const RPGIndex& layer) const {
	if (_variable_idx >= 0) { // If the predicate is 0-ary, there is no actual extension, we thus treat the case specially.
		return layer.is_true(_variable_idx);  // return true iff the constraint is satisfied, otherwise the CSP is unsolvable
	} else {
        LPT_DEBUG("heuristic", "Updating extension for " << *_term );
		return update(csp, translator, layer.get_extension(_term->getSymbolId()));
	}
}

bool ExtensionalConstraint::update(GecodeCSP& csp, const CSPTranslator& translator, const Gecode::TupleSet& extension) const {
	// Check whether the extension contains no tuples, then the CSP is unsolvable
    assert( extension.finalized() );
	if (extension.tuples() == 0 ) return false;

	// Collect the references to the CSP variables
	Gecode::IntVarArgs variables;
	for (unsigned csp_var_index:_subterm_variable_indexes) {
		variables << translator.resolveVariableFromIndex(csp_var_index, csp);
	}

	if (!_predicate) {
		variables << translator.resolveVariableFromIndex(_term_variable_index, csp);
	}

	// Post the constraint with the given extension

	Gecode::extensional(csp, variables, extension);
// 	Gecode::extensional(csp, variables, extension, Gecode::EPK_SPEED);

	LPT_DEBUG("heuristic", "Posted extensional constraint for term " << *_term << ":\n" << print::extensional(variables, extension));
	LPT_DEBUG("heuristic", "Resulting CSP is: " << csp);
	return true;
}

std::ostream& ExtensionalConstraint::print(std::ostream& os) const {
	os << "Extensional Constraint on term \"" << *_term << "\"" << std::endl;
	return os;
}

Gecode::TupleSet ExtensionalConstraint::compute_extension(unsigned symbol_id, const State& state) const {
	std::vector<unsigned> managed(ProblemInfo::getInstance().getNumLogicalSymbols(), 0);
	managed.at(symbol_id) = (unsigned) true;
	ExtensionHandler extension_handler(_tuple_index, managed);

	for (unsigned variable = 0, n = state.numAtoms(); variable < n; ++variable) {
		object_id value = state.getValue(variable);
		extension_handler.process_atom(variable, value);
	}

	return extension_handler.generate_extension(symbol_id);
}



} } // namespaces
