
#include <problem.hxx>
#include <problem_info.hxx>
#include <languages/fstrips/terms.hxx>
#include <languages/fstrips/formulae.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/gecode_csp.hxx>
#include <constraints/gecode/utils/term_list_iterator.hxx>


namespace fs0 { namespace gecode {


Gecode::IntVar Helper::createPlanningVariable(Gecode::Space& csp, VariableIdx variable) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	return createVariable(csp, info.getVariableType(variable));
}

Gecode::IntVar Helper::createTemporaryVariable(Gecode::Space& csp, TypeIdx typeId) {
	return createVariable(csp, typeId);
}

Gecode::IntVar Helper::createTemporaryIntVariable(Gecode::Space& csp, int min, int max) {
	return Gecode::IntVar(csp, min, max);
}

Gecode::BoolVar Helper::createBoolVariable(Gecode::Space& csp) {
	return Gecode::BoolVar(csp, 0, 1);
}

Gecode::IntVar Helper::createVariable(Gecode::Space& csp, TypeIdx typeId) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	auto type = info.get_type_id(typeId);

	if ( type == type_id::int_t ) {
		const auto& bounds = info.getTypeBounds(typeId);
		return Gecode::IntVar(csp, bounds.first, bounds.second);
	}
	else if ( type == type_id::bool_t ) {
		return Gecode::IntVar( csp, 0, 1 );
	}
	else {
		assert(type == type_id::object_t);
		const std::vector<object_id>& objects = info.getTypeObjects(typeId);
		const std::vector<int> values = fs0::values<int>(objects, ObjectTable::EMPTY_TABLE);
		return Gecode::IntVar(csp, Gecode::IntSet(values.data(), values.size())); // TODO - Check if we can change this for a range-like domain creation
	}
}

void Helper::constrainCSPVariable(GecodeCSP& csp, const Gecode::IntVar& variable, const Gecode::IntSet& domain) {
	if (domain.size() ==  static_cast<unsigned>(domain.max() - domain.min()) + 1) { // A micro-optimization
		Gecode::dom(csp, variable, domain.min(), domain.max());
	}
	Gecode::dom(csp, variable, domain);
}

Gecode::TupleSet Helper::extensionalize(const fs::StaticHeadedNestedTerm* term) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	auto f_data = info.getSymbolData(term->getSymbolId());
	const auto& functor = f_data.getFunction();

	Gecode::TupleSet tuples;

	for (term_list_iterator it(term->getSubterms()); !it.ended(); ++it) {
		try {
			object_id out = functor(it.arguments());
			type_id t = o_type(out);
			if (t == type_id::bool_t) {
				tuples.add(it.getIntArgsElement(fs0::value<bool>(out))); // Add the term value as the last element
			} else if (t == type_id::int_t || t == type_id::object_t) {
				tuples.add(it.getIntArgsElement(fs0::value<int>(out))); // Add the term value as the last element
			} else {
				throw std::runtime_error("Extensionalization of constraints not yet ready for objects of this type");
			}
		}
		catch(const std::out_of_range& e) {}  // If the functor produces an exception, we simply consider it non-applicable and go on.
		catch(const UndefinedValueAccess& e) {}
	}

	tuples.finalize();
	return tuples;
}

Gecode::TupleSet Helper::extensionalize(const fs::AtomicFormula* formula) {
	Gecode::TupleSet tuples;

	for (term_list_iterator it(formula->getSubterms()); !it.ended(); ++it) {
		try {
			if (formula->_satisfied(it.arguments())) {
				tuples.add(it.getIntArgsElement());
			}
		}
		catch(const std::out_of_range& e) {}  // If the functor produces an exception, we simply consider it non-applicable and go on.
		catch(const UndefinedValueAccess& e) {}
		
	}

	tuples.finalize();
	return tuples;
}

void Helper::postBranchingStrategy(GecodeCSP& csp) {
	// Beware that the order in which the branching strategies are posted matters.
	// For the integer variables, we post an unitialized value selector that will act as a default INT_VAL_MIN selector
	// until it is instructed (depending on the planner configuration) in order to favor lower-h_max atoms.
	Gecode::branch(csp, csp._intvars, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL(&Helper::value_selector));
	Gecode::branch(csp, csp._boolvars, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MIN());
}

int Helper::selectValueIfExists(Gecode::IntVarValues& value_set, int value) {
	assert(value_set());
	int arbitrary_element = 0;
	for (; value_set(); ++value_set) {
		arbitrary_element = value_set.val();
		if (arbitrary_element == value) return value;
	}
	return arbitrary_element;
}


int Helper::value_selector(const Gecode::Space& home, Gecode::IntVar x, int csp_var_idx) {
	// "A branch value function takes a constant reference to a space, a variable, and the variable’s
	// position and returns a value, where the type of the value depends on the variable type."
	return static_cast<const GecodeCSP&>(home).select_value(x, csp_var_idx);
}

} } // namespaces
