
#include <fs/core/constraints/gecode/supports.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>
#include <fs/core/constraints/gecode/gecode_csp.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/problem_info.hxx>

namespace fs0 { namespace gecode {

std::vector<AtomIdx>
Supports::extract_support(const GecodeCSP* solution, const CSPTranslator& translator, const std::vector<std::pair<unsigned, std::vector<unsigned>>>& tuple_indexes, const std::vector<AtomIdx>& necessary_tuples) {
	const auto& info = ProblemInfo::getInstance();
	const auto& tuple_index = Problem::getInstance().get_tuple_index();
	std::vector<AtomIdx> support;
	
	// We extract the actual support from the solution
	// First process the direct state variables
	for (const auto& element:translator.getAllInputVariables()) {
		VariableIdx variable = element.first;
		auto int_val = translator.resolveVariableFromIndex(element.second, *solution).val();
		object_id value = make_object(info.sv_type(variable), int_val);
		
		support.push_back(tuple_index.to_index(variable, value));
	}
	
	// Now the rest of fluent elements, i.e. the nested fluent terms
	for (const auto& tuple_info:tuple_indexes) {
		unsigned symbol = tuple_info.first;
		
		ValueTuple tuple;
		for (unsigned subterm_idx:tuple_info.second) {
			tuple.push_back(translator.resolveValueFromIndex(subterm_idx, *solution));
		}
		
		support.push_back(tuple_index.to_index(symbol, tuple));
	}
	
	// Now the support of atoms such as 'clear(b)' that might appear in formulas in non-negated form.
	support.insert(support.end(), necessary_tuples.begin(), necessary_tuples.end());
	
	return support;
}



} } // namespaces
