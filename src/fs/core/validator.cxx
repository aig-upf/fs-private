
#include <iostream>
#include <sstream>

#include <fs/core/validator.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/utils//binding_iterator.hxx>
#include <fs/core/utils//printers/vector.hxx>
#include <fs/core/utils//printers/helper.hxx>


namespace fs0 {

class InvalidProblemSpecification : public std::runtime_error {
public:
	InvalidProblemSpecification(const std::string& msg) : std::runtime_error(msg) {}
};

bool
Validator::validate_state(const State& state, const ProblemInfo& info) {
	bool error = false;
	unsigned num_atoms = state.numAtoms();

	// Check that the value assigned to each state variable is of the right type
	for (VariableIdx variable = 0; variable < num_atoms; ++variable) {
		if (! info.isBoundedType(info.getVariableType(variable)))
			continue;
		object_id value = state.getValue(variable);
		const auto& objects = info.getVariableObjects(variable);

		if (std::find(objects.begin(), objects.end(), value) == objects.end()) {
			std::cerr << "Value \"" << value << "\" assigned to state variable '" << info.getVariableName(variable) << "' is not of the correct type" << std::endl;
		}
	}

	// Check that the extension of each fluent symbol has been fully specified
	// TODO - THIS CHECK IS NOT CORRECT. There might be domain points for certain logical symbols
	// TODO - that are not modeled as state variables, as the preprocessing reachability analysis
	// TODO - might have shown that they might never be true
	/*
	for (unsigned symbol = 0; symbol < info.getNumLogicalSymbols(); ++symbol) {
		const SymbolData& data = info.getSymbolData(symbol);

		if (data.isStatic()) continue;
		// We have a fluent symbol

		for (utils::binding_iterator it(data.getSignature(), info); !it.ended(); ++it) {
			auto binding = *it;
			const ValueTuple& point = binding.get_full_binding();
			VariableIdx variable = info.resolveStateVariable(symbol, point);

			if (variable > num_atoms) {
				std::cerr << "Variable with index "<< variable
					<< ", corresponding to (fluent) symbol \"" << info.getSymbolName(symbol) << "\", not included in dynamic part of the state."
					<< " There must be some error." << std::endl;
				error = true;
			}
		}
	}
	*/
	return error;
}

bool
Validator::validate_problem_info(const ProblemInfo& info) {
	bool error = false;

	// Check that the extension of each static symbol has been fully specified
	for (unsigned symbol = 0; symbol < info.getNumLogicalSymbols(); ++symbol) {
		const SymbolData& data = info.getSymbolData(symbol);

		if (!data.isStatic()) continue;
		// Otherwise, we have a static symbol

		for (utils::binding_iterator it(data.getSignature(), info); !it.ended(); ++it) {
			auto binding = *it;
			const ValueTuple& point = binding.get_full_binding();

			try {
				object_id image = data.getFunction()(point);
				const auto& objects = info.getTypeObjects(data.getCodomainType());
				if (std::find(objects.begin(), objects.end(), image) == objects.end()) {
					std::cerr << "Value \"" << image << "\" assigned to symbol \"" << info.getSymbolName(symbol) << "\" at point " << print::container(print::Helper::name_objects(point)) << " is not of the correct type" << std::endl;
				}

			} catch (const std::out_of_range& e) {
				std::cerr << "The denotation of (static) symbol \"" << info.getSymbolName(symbol) << "\" is undefined at point " << print::container(print::Helper::name_objects(point)) << std::endl;
				error = true;
			} catch (const UndefinedValueAccess& e) {
				std::cerr << "The denotation of (static) symbol \"" << info.getSymbolName(symbol) << "\" is undefined at point " << print::container(print::Helper::name_objects(point)) << std::endl;
				error = true;
			}
		}
	}

	return error;
}

void
Validator::validate_problem(const Problem& problem, const ProblemInfo& info) {
	bool error = validate_problem_info(info);
	error |= validate_state(problem.getInitialState(), info);
	if (error) {
		throw InvalidProblemSpecification("Some errors were detected in the problem specification. Check standard error stream for details");
	}
}

} // namespaces
