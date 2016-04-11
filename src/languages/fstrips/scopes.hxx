
#pragma once

#include <fs0_types.hxx>

namespace fs0 {  class ActionBase; }

namespace fs0 { namespace language { namespace fstrips {

class Term;
class Formula;
class ActionEffect;
class FluentHeadedNestedTerm;


//! A number of helper methods to compute and deal with action / formula / term scopes
class ScopeUtils {
public:
	typedef std::set<const FluentHeadedNestedTerm*> TermSet;
	
	//! Computes the direct scope of a term, i.e. a vector with all state variables involved in the term.
	static std::vector<VariableIdx> computeDirectScope(const Term* term);
	static void computeDirectScope(const Term* term, std::set<VariableIdx>& scope);
	
	//! Computes the direct scope of an action effect term
	static std::vector<VariableIdx> computeDirectScope(const ActionEffect* effect);
	static void computeDirectScope(const ActionEffect* effect, std::set<VariableIdx>& scope);
	
	//! Computes the indirect scope of an effect, i.e. the set of all the state variables involved in the effect that are produced by nested fluents
	static std::vector<const FluentHeadedNestedTerm*> computeIndirectScope(const ActionEffect* effect);
	static void computeIndirectScope(const ActionEffect* effect, TermSet& scope);
	
	//! Computes the direct scope of a formula
	static std::vector<VariableIdx> computeDirectScope(const Formula* formula);
	static void computeDirectScope(const Formula* formula, std::set<fs0::VariableIdx>& scope);
	
	//! Computes the indirect scope of a formula
	static void computeIndirectScope(const Formula* formula, ScopeUtils::TermSet& scope);
	
	//! Returns the direct scope of an action, i.e. the set of all the state variables that are directly relevant
	//! to either the preconditions or some effect of the action.
	//! This excludes indirect relevant actions produced by nested fluents.
	static std::vector<VariableIdx> computeActionDirectScope(const ActionBase& action);
	
	//! Computes all the (indirect) state variables in which a nested fluent might result
	// TODO - Check if really necessary
	static void computeIndirectScope(FluentHeadedNestedTerm& nested, std::set<VariableIdx>& scope);

	//!
	static void computeIndirectTermScope(const Term* term, std::set<VariableIdx>& scope);

	//!
	static std::vector<Atom> compute_affected_atoms(const ActionEffect* effect);
	
	//! Adds to 'variables' all those state variables that can be derived from the given element (formula / term),
	//! including variables which are directly present and those that are present through nested terms.
	//! It DOES NOT take into account "predicative" state variables, since these are not considered state variables anymore.
	static void computeRelevantElements(const Term* element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols);
	static void computeRelevantElements(const Formula* element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols);
};

} } } // namespaces
