
#include <constraints/gecode/utils/extensional_constraint.hxx>
#include <problem.hxx>
#include <state.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/utils/translation.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <utils/logging.hxx>

namespace fs0 { namespace gecode {

ExtensionalConstraint::ExtensionalConstraint(const fs::FluentHeadedNestedTerm* term, bool predicate)
	: _predicate(predicate), _term(term), _variable_idx(-1)
{}

void ExtensionalConstraint::register_variables(GecodeCSPVariableTranslator& translator) {
	// No variables as of now
}

void ExtensionalConstraint::register_constraints(GecodeCSPVariableTranslator& translator) {
	FDEBUG("translation", "Preprocessing extensional constraints for term " << *_term);
	const ProblemInfo& info = Problem::getInfo();
	
	for (auto subterm:_term->getSubterms()) {
		_subterm_variable_indexes.push_back(translator.resolveVariableIndex(subterm, CSPVariableType::Input));
	}
	
	if (!_predicate) _term_variable_index = translator.resolveVariableIndex(_term, CSPVariableType::Input);
	
	if (_term->getSubterms().empty()) {
		_variable_idx = info.resolveStateVariable(_term->getSymbolId(), {});
	}
}

bool ExtensionalConstraint::update(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const State& state) const {
	assert(0); // We need to obtain the actual extension from the state - is this method necessary, though??
// 	update(csp, translator, layer.get_extension(_term->getSymbolId()));
}

bool ExtensionalConstraint::update(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const GecodeRPGLayer& layer) const {
	if (_variable_idx >= 0) { // If the predicate is 0-ary, there is no actual extension, we thus treat the case specially.
		const auto& domain = layer.get_index_domain(_variable_idx);
		return (domain.find(1) != domain.end()); // return true iff the constraint is satisfied, otherwise the CSP is unsolvable
	} else {
		return update(csp, translator, layer.get_extension(_term->getSymbolId()));
	}
}

bool ExtensionalConstraint::update(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const Gecode::TupleSet& extension) const {
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
	// FDEBUG("translation", "Posted extensional constraint:" << print::extensional(variables, extension));
	return true;
}

} } // namespaces
