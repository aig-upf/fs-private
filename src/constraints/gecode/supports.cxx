
#include <constraints/gecode/supports.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <utils/tuple_index.hxx>
#include <problem.hxx>

namespace fs0 { namespace gecode {

std::vector<TupleIdx> Supports::extract_support(const SimpleCSP* solution, const GecodeCSPVariableTranslator& translator, const std::vector<std::pair<unsigned, std::vector<unsigned>>>& tuple_indexes) {
	const auto& tuple_index = Problem::getInstance().get_tuple_index();
	std::vector<TupleIdx> support;
	
	// We extract the actual support from the solution
	// First process the direct state variables
	for (const auto& element:translator.getAllInputVariables()) {
		VariableIdx variable = element.first;
		ObjectIdx value = translator.resolveVariableFromIndex(element.second, *solution).val();
		
		unsigned tuple_idx = tuple_index.to_index(Atom(variable, value));
		support.push_back(tuple_idx);
	}
	
	// Now the rest of fluent elements
	for (const auto& tuple_info:tuple_indexes) {
		unsigned symbol = tuple_info.first;
		
		ValueTuple tuple;
		for (unsigned subterm_idx:tuple_info.second) {
			tuple.push_back(translator.resolveValueFromIndex(subterm_idx, *solution));
		}
		
		unsigned tuple_idx = tuple_index.to_index(symbol, tuple);
		support.push_back(tuple_idx);
	}
	
	return support;
}

} } // namespaces
