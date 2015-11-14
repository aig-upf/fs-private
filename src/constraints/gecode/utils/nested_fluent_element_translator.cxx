
#include <problem.hxx>
#include <utils/logging.hxx>
#include <utils/printers/gecode.hxx>
#include <constraints/gecode/utils/nested_fluent_element_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <constraints/gecode/base.hxx>

namespace fs0 { namespace gecode {

NestedFluentElementTranslator::NestedFluentElementTranslator(fs::FluentHeadedNestedTerm::cptr term, const VariableCounter& counter)
	: _term(term), _counter(counter)
{}


void NestedFluentElementTranslator::register_variables(CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	FDEBUG( "translation", "Registering nested fluent CSP variables " << *_term);
	SimpleCSP& csp = translator.getBaseCSP();
	
	// Now, for a nested fluent such as e.g. 'tile(blank())', we register as involved state variables all state variables
	// tile(nw), tile(n), ..., etc., where nw, n, etc. are the possible values for state variable 'blank()'.
	unsigned table_index = 0;
	for (nested_fluent_iterator it(_term); !it.ended(); ++it) {
		VariableIdx variable = it.getDerivedStateVariable();
		FDEBUG( "translation", "Registering derived state variable " << Problem::getInfo().getVariableName(variable));
		translator.registerInputStateVariable(variable, _counter.is_nullable(variable));
		
		if (_counter.is_nullable(variable)) {
			_fluent_data.addTableReificationVariable(translator.create_bool_variable());
			_fluent_data.addIndexReificationVariable(translator.create_bool_variable());
		} else {
			_fluent_data.addTableReificationVariable(-1);
			_fluent_data.addIndexReificationVariable(-1);
		}
		
		++table_index;
	}
	
	// Create a CSP variable that will be the element constraint index.
	_fluent_data.setIndex(translator.add_intvar(Helper::createTemporaryIntVariable(csp, 0, table_index)));

	// Finally register the "standard" temporary variable for the term root.
	translator.registerNestedTerm(_term, type);
}

void NestedFluentElementTranslator::register_constraints(CSPVariableType type, GecodeCSPVariableTranslator& translator) {
	SimpleCSP& csp = translator.getBaseCSP();

	const std::vector<fs::Term::cptr>& subterms = _term->getSubterms();

	FDEBUG("translation", "Registering constraints for fluent nested term " << *_term << (type == CSPVariableType::Output ? "'" : ""));

	Gecode::IntVarArgs table; // The actual array of variables that will form the element constraint table

	const Gecode::IntVar& zero_based_index = _fluent_data.getIndex(csp);
	std::vector<VariableIdx>& table_variables = _fluent_data.getTableVariables();
	
	Gecode::TupleSet correspondence; // The correspondence between the index variable possible values and their 0-indexed position in the element constraint table
	for (nested_fluent_iterator it(_term); !it.ended(); ++it) {
		VariableIdx variable = it.getDerivedStateVariable();
		table << translator.resolveInputStateVariable(csp, variable);
		table_variables.push_back(variable);

		correspondence.add(it.getIntArgsElement());
	}
	correspondence.finalize();

	// Post the extensional constraint relating the value of the subterm variables to that of the temporary 0..m index variable
	Gecode::IntVarArgs index_variables = translator.resolveVariables(subterms, CSPVariableType::Input, csp);
	index_variables << zero_based_index;
	Gecode::extensional(csp, index_variables, correspondence);
	FDEBUG("translation", "Fluent-headed term \"" << *_term << "\" produces indexing constraint: " << print::extensional(index_variables, correspondence));

	// Now post the actual element constraint
	const Gecode::IntVar& element_result = translator.resolveVariable(_term, type, csp);
	Gecode::element(csp, table, zero_based_index, element_result);
	FDEBUG("translation", "Fluent-headed term \"" << *_term << "\" produces element constraint: " << print::element(table, zero_based_index, element_result));
	
	if (_counter.useDontCare()) {
		register_dont_care_constraints(translator);
	}
}


void NestedFluentElementTranslator::register_dont_care_constraints(GecodeCSPVariableTranslator& translator) {
	SimpleCSP& csp = translator.getBaseCSP();

	FDEBUG("translation", "Registering DON'T CARE optimization constraints for fluent nested term " << *_term);

	const Gecode::IntVar& zero_based_index = _fluent_data.getIndex(csp);
	
	for (nested_fluent_iterator it(_term); !it.ended(); ++it) {
		VariableIdx variable = it.getDerivedStateVariable();
		
		// If for some reason the variable cannot be set to DON'T CARE, we cannot post the constraints below for it
		if (!_counter.is_nullable(variable)) continue;
		
		unsigned iteration_index = it.getIndex();
		auto gecode_variable = translator.resolveInputStateVariable(csp, variable);
		
		auto& idx_reification = _fluent_data.getIndexReificationVariable(csp, iteration_index);
		auto& tab_reification = _fluent_data.getTableReificationVariable(csp, iteration_index);
		
		// Post the necessary reification constraints to achieve the expression IDX = i \lor f(IDX) = DONT_CARE
		Gecode::rel(csp, zero_based_index, Gecode::IRT_EQ, iteration_index, idx_reification); // IDX = i <=> b0
		Gecode::rel(csp, gecode_variable, Gecode::IRT_EQ, DONT_CARE::get(), tab_reification); // f(IDX) = DONT_CARE <=> b1
		Gecode::rel(csp, idx_reification, Gecode::BOT_OR, tab_reification, 1); // b0 \lor b1
	}
}






} } // namespaces
