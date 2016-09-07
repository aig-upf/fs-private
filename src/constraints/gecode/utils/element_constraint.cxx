
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/utils/element_constraint.hxx>

#include <problem.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/gecode.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/handlers/base_csp.hxx>
#include <constraints/gecode/utils/nested_fluent_iterator.hxx>

namespace fs0 { namespace gecode {

NestedFluentElementTranslator::NestedFluentElementTranslator(const fs::FluentHeadedNestedTerm* term)
	: _term(term)
{}


void NestedFluentElementTranslator::register_variables(CSPTranslator& translator) {
	LPT_DEBUG( "translation", "Registering nested fluent CSP variables " << *_term);
	GecodeCSP& csp = translator.getBaseCSP();
	
	// Now, for a nested fluent such as e.g. 'tile(blank())', we register as involved state variables all state variables
	// tile(nw), tile(n), ..., etc., where nw, n, etc. are the possible values for state variable 'blank()'.
	unsigned table_index = 0;
	for (nested_fluent_iterator it(_term); !it.ended(); ++it) {
		VariableIdx variable = it.getDerivedStateVariable();
		LPT_DEBUG( "translation", "Registering derived state variable " << ProblemInfo::getInstance().getVariableName(variable));
		translator.registerInputStateVariable(variable);
		
		++table_index;
	}
	
	// Create a CSP variable that will be the element constraint index.
	_fluent_data.setIndex(translator.add_intvar(Helper::createTemporaryIntVariable(csp, 0, table_index-1)));

	// Finally register the "standard" temporary variable for the term root.
	translator.registerNestedTerm(_term);
}

void NestedFluentElementTranslator::register_constraints(CSPTranslator& translator) {
	GecodeCSP& csp = translator.getBaseCSP();

	const std::vector<const fs::Term*>& subterms = _term->getSubterms();

	LPT_DEBUG("translation", "Registering constraints for fluent nested term " << *_term);

	Gecode::IntVarArgs table; // The actual array of variables that will form the element constraint table

	const Gecode::IntVar& zero_based_index = _fluent_data.getIndex(csp);
	std::vector<VariableIdx>& table_variables = _fluent_data.getTableVariables();
	
	Gecode::TupleSet correspondence; // The correspondence between the index variable possible values and their 0-indexed position in the element constraint table
	for (nested_fluent_iterator it(_term); !it.ended(); ++it) {
		VariableIdx variable = it.getDerivedStateVariable();
		table << translator.resolveInputStateVariable(csp, variable);
		table_variables.push_back(variable);

		correspondence.add(it.getIndexedIntArgsElement());
	}
	correspondence.finalize();

	// Post the extensional constraint relating the value of the subterm variables to that of the temporary 0..m index variable
	Gecode::IntVarArgs index_variables = translator.resolveVariables(subterms, csp);
	index_variables << zero_based_index;
	Gecode::extensional(csp, index_variables, correspondence);
	LPT_DEBUG("translation", "Fluent-headed term \"" << *_term << "\" produces indexing constraint: " << print::extensional(index_variables, correspondence));

	// Now post the actual element constraint
	const Gecode::IntVar& element_result = translator.resolveVariable(_term, csp);
	Gecode::element(csp, table, zero_based_index, element_result);
	LPT_DEBUG("translation", "Fluent-headed term \"" << *_term << "\" produces element constraint: " << print::element(table, zero_based_index, element_result));
}

} } // namespaces
