
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/terms.hxx>
#include <fs/core/constraints/gecode/utils/extensional_constraint.hxx>
#include <fs/core/state.hxx>
#include <fs/core/constraints/gecode/gecode_space.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>
#include <fs/core/constraints/gecode/extensions.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/utils/printers/gecode.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>

namespace fs0::gecode {

ExtensionalConstraint::ExtensionalConstraint(const fs::FluentHeadedNestedTerm* term, const AtomIndex& tuple_index, bool predicate, bool negative)
	: _negative(negative), _predicate(predicate), _term(term), _variable_idx(-1), _tuple_index(tuple_index)
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
bool ExtensionalConstraint::update(FSGecodeSpace& csp, const CSPTranslator& translator, const State& state) const {
	if (_variable_idx >= 0) { // If the predicate is 0-ary, there is no actual extension, we thus treat the case specially.
	    auto intval = unsigned(state.getValue(_variable_idx));
        assert(intval == 1 || intval == 0);
		return bool(intval) == !_negative;
	} else {
		return update(csp, translator, compute_extension(_term->getSymbolId(), state));
	}
}

bool ExtensionalConstraint::update(FSGecodeSpace& csp, const CSPTranslator& translator, const RPGIndex& layer) const {
	if (_variable_idx >= 0) { // If the predicate is 0-ary, there is no actual extension, we thus treat the case specially.
	    bool sat = layer.is_true(_variable_idx);  // return true iff the constraint is satisfied, otherwise the CSP is unsolvable
		return sat == !_negative;
	} else {
		LPT_EDEBUG("heuristic", "Updating extension for " << *_term );
		return update(csp, translator, layer.get_extension(_term->getSymbolId()));
	}
}

bool ExtensionalConstraint::update(FSGecodeSpace& csp, const CSPTranslator& translator, const Gecode::TupleSet& extension) const {
    assert(extension.finalized());

    // If the extension of the constraint is empty and it is not a negative constraint, then
    // we can already flag the CSP as unsolvable
	if (!_negative && extension.tuples() == 0) return false;

	// Collect the references to the CSP variables
	Gecode::IntVarArgs variables;
	for (unsigned csp_var_index:_subterm_variable_indexes) {
		variables << translator.resolveVariableFromIndex(csp_var_index, csp);
	}

	if (!_predicate) {
		variables << translator.resolveVariableFromIndex(_term_variable_index, csp);
	}

	// Post the constraint with the given extension
	Gecode::extensional(csp, variables, extension, !_negative);

	LPT_EDEBUG("heuristic", "Posted extensional constraint for term " << *_term << ":\n" << print::extensional(variables, extension));
	LPT_EDEBUG("heuristic", "Resulting CSP is: " << csp);
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



} // namespaces
