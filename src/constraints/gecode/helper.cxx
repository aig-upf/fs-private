
#include <constraints/gecode/helper.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <problem.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <relaxed_state.hxx>
#include <utils/cartesian_iterator.hxx>


namespace fs0 { namespace gecode {
	
	
Gecode::IntVar Helper::createPlanningVariable(Gecode::Space& csp, VariableIdx variable) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	return createVariable(csp, info.getVariableType(variable));
}

Gecode::IntVar Helper::createTemporaryVariable(Gecode::Space& csp, TypeIdx typeId) {
	return createVariable(csp, typeId);
}

Gecode::IntVar Helper::createTemporaryIntVariable(Gecode::Space& csp, int min, int max) {
	return Gecode::IntVar(csp, min, max);
}

Gecode::IntVar Helper::createVariable(Gecode::Space& csp, TypeIdx typeId) {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	auto generic_type = info.getGenericType(typeId);
	
	if ( generic_type == ProblemInfo::ObjectType::INT ) {
		const auto& bounds = info.getTypeBounds(typeId);
		return Gecode::IntVar( csp, bounds.first, bounds.second );
	}
	else if ( generic_type == ProblemInfo::ObjectType::BOOL ) {
		return Gecode::IntVar( csp, 0, 1 );
	}
	else {
		assert(generic_type == ProblemInfo::ObjectType::OBJECT);
		const ObjectIdxVector& values = info.getTypeObjects(typeId);
		return Gecode::IntVar(csp, Gecode::IntSet(values.data(), values.size()));
	}
}

void Helper::constrainCSPVariable(SimpleCSP& csp, unsigned csp_variable_id, const DomainPtr& domain) {
	const Gecode::IntVar& variable = csp._X[csp_variable_id];
	
	if ( domain->size() == 1 ) { // The simplest case
		Gecode::rel(csp, variable, Gecode::IRT_EQ, *(domain->cbegin()));
	} else {
		ObjectIdx lb = *(domain->cbegin());
		ObjectIdx ub = *(domain->crbegin());
		
		if (domain->size() == static_cast<unsigned>(ub - lb) ) { // MRJ: Check this is a safe assumption - We can guarantee it is unsigned, since the elements in the domain are ordered
			Gecode::dom(csp, variable, lb, ub); // MRJ: lb <= variable <= ub
		} else { // TODO - MRJ: worst case (performance wise) yet I think it can be optimised in a number of ways
			// We constraint the variable through an extensional constraint
			Gecode::extensional(csp, IntVarArgs() << variable, buildTupleset(*domain));
		}
	}
}


Gecode::TupleSet Helper::buildTupleset(const fs0::Domain& domain) {
	Gecode::TupleSet tuples;
	for (auto value:domain) {
		tuples.add(IntArgs(1, value));
	}
	tuples.finalize();
	return tuples;
}

//! @deprecated
/* 
Gecode::TupleSet Helper::extensionalize(const Term::cptr term, const VariableIdxVector& scope) {
	if (scope.size() > 2) throw std::runtime_error("Error trying to extensionalize a term with too high a scope");
	
	// This is not entirely correct - we need to compile static fluents differently
	// We need to examine subterm by subterm, fix those that are fixed (constants), and use whole domains for those that are either 
	// state variables or temporary variables.
	assert(0); 
	
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	Gecode::TupleSet tuples;
	
	if (scope.size() == 1) {
		
		for(ObjectIdx value:info.getVariableObjects(scope[0])) {
			ObjectIdx out = term->interpret(Projections::zip(scope, {value}));
			tuples.add(Gecode::IntArgs(2, value, out));
		}
		
	} else { // scope.size() == 2
		
		for(ObjectIdx x:info.getVariableObjects(scope[0])) {
			for(ObjectIdx y:info.getVariableObjects(scope[1])) {
				ObjectIdx out = term->interpret(Projections::zip(scope, {x, y}));
				tuples.add(Gecode::IntArgs(3, x, y, out));
			}
		}
	}
	
	tuples.finalize();
	return tuples;
}
*/

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
	branch(csp, csp._X, INT_VAR_SIZE_MIN(), INT_VAL_MIN()); // TODO posting a particular branching strategy might make sense to prioritize some branching strategy?
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

} } // namespaces
