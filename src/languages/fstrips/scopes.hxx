
#pragma once
#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>

namespace fs0 { 
	class GroundAction;
}

namespace fs0 { namespace language { namespace fstrips {

//! A term that has not yet been processed, meaning that it might possibly contain references to yet-unresolved values of action parameters,
//! non-consolidated state variables, etc.
class ScopeUtils {
public:
	typedef std::set<FluentHeadedNestedTerm::cptr> TermSet;
	
	//! Computes the direct scope of a term, i.e. a vector with all state variables involved in the term.
	static std::vector<VariableIdx> computeDirectScope(Term::cptr term);
	static void computeDirectScope(Term::cptr term, std::set<VariableIdx>& scope);
	
	//! Computes the direct scope of an action effect term
	static std::vector<VariableIdx> computeDirectScope(ActionEffect::cptr effect);
	static void computeDirectScope(ActionEffect::cptr effect, std::set<VariableIdx>& scope);
	
	//! Computes the indirect scope of an effect, i.e. the set of all the state variables involved in the effect that are produced by nested fluents
	static std::vector<FluentHeadedNestedTerm::cptr> computeIndirectScope(ActionEffect::cptr effect);
	static void computeIndirectScope(ActionEffect::cptr effect, TermSet& scope);
	
	//! Computes the direct scope of a formula
	static std::vector<VariableIdx> computeDirectScope(AtomicFormula::cptr formula);
	static void computeDirectScope(AtomicFormula::cptr formula, std::set<VariableIdx>& scope);
	
	//! Computes the indirect scope of a formula
	static void computeIndirectScope(AtomicFormula::cptr formula, TermSet& scope);
	
	//! Returns the direct scope of an action, i.e. the set of all the state variables that are directly relevant
	//! to either the preconditions or some effect of the action.
	//! This excludes indirect relevant actions produced by nested fluents.
	static std::vector<VariableIdx> computeActionDirectScope(const GroundAction& action);
	
	//! Computes all the (indirect) state variables in which a nested fluent might result
	// TODO - Check if really necessary
	static void computeIndirectScope(FluentHeadedNestedTerm& nested, std::set<VariableIdx>& scope);


};

} } } // namespaces
