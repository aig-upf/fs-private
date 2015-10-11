

#pragma once

#include <fs0_types.hxx>
#include <problem_info.hxx>
#include <constraints/direct/compiled.hxx>
#include <languages/fstrips/language.hxx>
#include <gecode/int.hh>

using namespace Gecode;

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class SimpleCSP; class GecodeCSPVariableTranslator; enum class CSPVariableType;

//! Some helper methods related to the construction of Gecode CSPs.
class Helper {
public:
	//! Creates a CSP variable constraining its domain according to the given type
	static Gecode::IntVar createVariable(Gecode::Space& csp, TypeIdx typeId, bool nullable = false);
	
	//! Creates a CSP variable constraining its domain based on the planning problem variable type
	static Gecode::IntVar createPlanningVariable(Gecode::Space& csp, VariableIdx var, bool nullable = false);
	
	//! Creates a CSP variable constraining its domain based on the given domain type
	static Gecode::IntVar createTemporaryVariable(Gecode::Space& csp, TypeIdx typeId);
	
	//! Creates a Gecode int variable within the given bounds.
	static Gecode::IntVar createTemporaryIntVariable(Gecode::Space& csp, int min, int max);
	
	//! Creates a Gecode bool variable within the given bounds.
	static Gecode::BoolVar createBoolVariable(Gecode::Space& csp);
	
	//! Constrains the given CSP variable to have values in the given domain
	static void constrainCSPVariable(SimpleCSP& csp, const Gecode::IntVar& variable, const Gecode::IntSet& domain, bool include_dont_care = false);
	
	//! Extensionalize a given (static) term by building a tupleset characterizing the (functional) relation
	//! that underlies the static term in all interpretations.
	static Gecode::TupleSet extensionalize(const fs::StaticHeadedNestedTerm::cptr term);
	
	// Build an extensional gecode tupleset with all those tuples that render a certain symbol true
	static Gecode::TupleSet extensionalize(const std::string& symbol);

	//! Builds a gecode tupleset from the values contained in a state variable domain
	static Gecode::TupleSet buildTupleset(const fs0::Domain& domain, bool include_dont_care = false);
	
	//! A simple helper to post a certain Gecode branching strategy to the CSP
	static void postBranchingStrategy(SimpleCSP& csp);
	
	//! Small helper to check whether a Gecode IntVarValues set contains a given value
	//! Unfortunately, it has linear cost.
	static int selectValueIfExists(IntVarValues& value_set, int value);
	
	//! Computes a proper DONT_CARE value which is as close as possible to zero but not used as the ID of any object.
	static int computeDontCareValue();
	
	static Gecode::IntSet add_dont_care(const IntSet& domain);
};

} } // namespaces