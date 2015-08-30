
#include <constraints/gecode/helper.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <problem.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include "base.hxx"
#include <constraints/gecode/csp_translator.hxx>
#include <relaxed_state.hxx>
#include <utils/cartesian_iterator.hxx>


namespace fs0 { namespace gecode {
	
	
Gecode::IntVar Helper::createPlanningVariable(Gecode::Space& csp, VariableIdx variable, bool nullable) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	return createVariable(csp, info.getVariableType(variable), nullable);
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

Gecode::IntVar Helper::createVariable(Gecode::Space& csp, TypeIdx typeId, bool nullable) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	auto generic_type = info.getGenericType(typeId);
	
	if ( generic_type == ProblemInfo::ObjectType::INT ) {
		assert(!nullable); // TODO
		const auto& bounds = info.getTypeBounds(typeId);
		Gecode::IntSet domain(bounds.first, bounds.second);
		return Gecode::IntVar(csp, domain);
	}
	else if ( generic_type == ProblemInfo::ObjectType::BOOL ) {
		assert(!nullable);
		return Gecode::IntVar( csp, 0, 1 );
	}
	else {
		assert(generic_type == ProblemInfo::ObjectType::OBJECT);
		ObjectIdxVector values = info.getTypeObjects(typeId); // We copy the vector so that we can add a DONT_CARE value if necessary
		if (nullable) values.push_back(DONT_CARE);
		return Gecode::IntVar(csp, Gecode::IntSet(values.data(), values.size())); // TODO - Check if we can change this for a range-like domain creation
	}
}

void Helper::constrainCSPVariable(SimpleCSP& csp, unsigned csp_variable_id, const DomainPtr& domain, bool include_dont_care) {
	const Gecode::IntVar& variable = csp._intvars[csp_variable_id];
	
	if (!include_dont_care && domain->size() == 1 ) { // The simplest case
		Gecode::rel(csp, variable, Gecode::IRT_EQ, *(domain->cbegin()));
	} else {
		ObjectIdx lb = *(domain->cbegin());
		ObjectIdx ub = *(domain->crbegin());
		
		if (!include_dont_care && domain->size() == static_cast<unsigned>(ub - lb) ) { // MRJ: Check this is a safe assumption - We can guarantee it is unsigned, since the elements in the domain are ordered
			Gecode::dom(csp, variable, lb, ub); // MRJ: lb <= variable <= ub
		} else { // TODO - MRJ: worst case (performance wise) yet I think it can be optimised in a number of ways
			// We constraint the variable through an extensional constraint
			Gecode::extensional(csp, IntVarArgs() << variable, buildTupleset(*domain, include_dont_care));
		}
	}
}

Gecode::TupleSet Helper::buildTupleset(const fs0::Domain& domain, bool include_dont_care) {
	Gecode::TupleSet tuples;
	for (auto value:domain) {
		tuples.add(IntArgs(1, value));
	}
	if (include_dont_care) tuples.add(IntArgs(1, DONT_CARE));
	tuples.finalize();
	return tuples;
}

Gecode::TupleSet Helper::extensionalize(const fs::StaticHeadedNestedTerm::cptr term) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	auto f_data = info.getFunctionData(term->getSymbolId());
	const Signature& signature = f_data.getSignature();
	const auto& functor = f_data.getFunction();
	
	Gecode::TupleSet tuples;
	
	utils::cartesian_iterator all_values(info.getSignatureValues(signature));
	for (; !all_values.ended(); ++all_values) {
		try {
			ObjectIdx out = functor(*all_values);
			tuples.add(Gecode::IntArgs(*all_values) << out); // Add the term value as the last element
		} catch(const std::out_of_range& e) {}  // If the functor produces an exception, we simply consider it non-applicable and go on.
	}
	
	tuples.finalize();
	return tuples;
}

void Helper::postBranchingStrategy(SimpleCSP& csp) {
	branch(csp, csp._intvars, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
	branch(csp, csp._boolvars, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
}

int Helper::selectValueIfExists(IntVarValues& value_set, int value) {
	assert(value_set());
	int arbitrary_element;
	for (; value_set(); ++value_set) {
		arbitrary_element = value_set.val();
		if (arbitrary_element == value) return value;
	}
	return arbitrary_element;
}

void Helper::update_csp(SimpleCSP& csp, const IntVarArgs& intvars, const BoolVarArgs& boolvars) {
	IntVarArray intarray(csp, intvars);
	csp._intvars.update(csp, false, intarray);
	
	BoolVarArray boolarray(csp, boolvars);
	csp._boolvars.update(csp, false, boolarray);	
}



} } // namespaces
