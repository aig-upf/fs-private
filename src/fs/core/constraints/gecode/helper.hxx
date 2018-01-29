

#pragma once

#include <fs/core/languages/fstrips/language_fwd.hxx>
#include <fs/core/fs_types.hxx>
#include <gecode/int.hh>

namespace fs0 { namespace gecode {

class GecodeCSP; class CSPTranslator;

//! Some helper methods related to the construction of Gecode CSPs.
class Helper {
public:
	static Helper& instance() {
		static Helper instance;
		return instance;
	}

	Helper();
	Helper(const Helper&)         = delete;
	void operator=(const Helper&) = delete;

	//! Creates a CSP variable constraining its domain according to the given type
	static Gecode::IntVar createVariable(Gecode::Space& csp, TypeIdx typeId);
	
	//! Creates a CSP variable constraining its domain based on the planning problem variable type
	static Gecode::IntVar createPlanningVariable(Gecode::Space& csp, VariableIdx var);
	
	//! Creates a CSP variable constraining its domain based on the given domain type
	static Gecode::IntVar createTemporaryVariable(Gecode::Space& csp, TypeIdx typeId);
	
	//! Creates a Gecode int variable within the given bounds.
	static Gecode::IntVar createTemporaryIntVariable(Gecode::Space& csp, int min, int max);
	
	//! Creates a Gecode bool variable within the given bounds.
	static Gecode::BoolVar createBoolVariable(Gecode::Space& csp);
	
	//! Constrains the given CSP variable to have values in the given domain
	static void constrainCSPVariable(GecodeCSP& csp, const Gecode::IntVar& variable, const Gecode::IntSet& domain);
	
	//! Extensionalize a given (static) term by building a tupleset characterizing the (functional) relation
	//! that underlies the static term in all interpretations.
	static Gecode::TupleSet extensionalize(const fs::StaticHeadedNestedTerm* term);
	
	//! Extensionalize any given formula
	static Gecode::TupleSet extensionalize(const fs::AtomicFormula* formula);

	//! A simple helper to post a certain Gecode branching strategy to the CSP
	static void postBranchingStrategy(GecodeCSP& csp);
	
	//! Small helper to check whether a Gecode IntVarValues set contains a given value
	//! Unfortunately, it has linear cost.
	static int selectValueIfExists(Gecode::IntVarValues& value_set, int value);
	
	//! A helper method to install our desired value-selection brancher
	static int value_selector(const Gecode::Space& home, Gecode::IntVar x, int csp_var_idx);

    //!
    const Gecode::TupleSet& compute_symbol_extension(unsigned symbol);
protected:
	std::vector<bool> _cached_static_extensions;
	std::vector<Gecode::TupleSet> _static_extensions;
};

} } // namespaces