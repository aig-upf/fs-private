
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
	const ProblemInfo& info = Problem::getInfo();
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
	const ProblemInfo& info = Problem::getInfo();
	auto generic_type = info.getGenericType(typeId);

	if ( generic_type == ProblemInfo::ObjectType::INT ) {
		const auto& bounds = info.getTypeBounds(typeId);
		if (!nullable) {
			return Gecode::IntVar(csp, bounds.first, bounds.second);
		} else {
			// We specify to possible ranges: the "normal" integer range plus a second range only with the DONT_CARE value
			const int ranges[2][2] = {{bounds.first, bounds.second}, {DONT_CARE::get(), DONT_CARE::get()}};
			return Gecode::IntVar(csp, Gecode::IntSet(ranges, 2));
		}
	}
	else if ( generic_type == ProblemInfo::ObjectType::BOOL ) {
		assert(!nullable);
		return Gecode::IntVar( csp, 0, 1 );
	}
	else {
		assert(generic_type == ProblemInfo::ObjectType::OBJECT);
		ObjectIdxVector values = info.getTypeObjects(typeId); // We copy the vector so that we can add a DONT_CARE value if necessary
		if (nullable) values.push_back(DONT_CARE::get());
		return Gecode::IntVar(csp, Gecode::IntSet(values.data(), values.size())); // TODO - Check if we can change this for a range-like domain creation
	}
}

void Helper::constrainCSPVariable(fs0::gecode::SimpleCSP& csp, const Gecode::IntVar& variable, const IntSet& domain, bool include_dont_care) {
	if (include_dont_care) {
		assert(0); // TODO 
// 		Gecode::extensional(csp, IntVarArgs() << variable, buildTupleset(*domain, include_dont_care));
		Gecode::dom(csp, variable, domain);
	} else {
		if (domain.size() ==  static_cast<unsigned>(domain.max() - domain.min()) + 1) { // A micro-optimization
			Gecode::dom(csp, variable, domain.min(), domain.max());
		}
		Gecode::dom(csp, variable, domain);
	}
}

Gecode::TupleSet Helper::buildTupleset(const fs0::Domain& domain, bool include_dont_care) {
	Gecode::TupleSet tuples;
	for (auto value:domain) {
		tuples.add(IntArgs(1, value));
	}
	if (include_dont_care) tuples.add(IntArgs(1, DONT_CARE::get()));
	tuples.finalize();
	return tuples;
}

Gecode::TupleSet Helper::extensionalize(const fs::StaticHeadedNestedTerm::cptr term) {
	const ProblemInfo& info = Problem::getInfo();
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

Gecode::TupleSet Helper::extensionalize(const std::string& symbol) {
	const ProblemInfo& info = Problem::getInfo();
	
	auto f_data = info.getFunctionData(info.getFunctionId(symbol));
	const Signature& signature = f_data.getSignature();
	const auto& functor = f_data.getFunction();

	Gecode::TupleSet tuples;

	utils::cartesian_iterator all_values(info.getSignatureValues(signature));
	for (; !all_values.ended(); ++all_values) {
		try {
			if (functor(*all_values)) {
				tuples.add(Gecode::IntArgs(*all_values));
			}
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
	int arbitrary_element = 0;
	for (; value_set(); ++value_set) {
		arbitrary_element = value_set.val();
		if (arbitrary_element == value) return value;
	}
	return arbitrary_element;
}


int Helper::computeDontCareValue() {
	const ProblemInfo& info = Problem::getInfo();
	int min = -1;
	int max = std::numeric_limits<int>::min();
	for (std::vector<ObjectIdx> objs:info.getTypeObjects()) {
		if (objs.empty()) continue;
		min = std::min(min, *std::min_element( std::begin(objs), std::end(objs) ));
		max = std::max(max, *std::max_element( std::begin(objs), std::end(objs) ));
	}
	// We select the closest value to zero which is not being used as the ID of some object / range integer
	return -1*max < min ? min : max;
}



} } // namespaces
