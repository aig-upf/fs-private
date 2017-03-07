
#pragma once

#include <fs_types.hxx>

namespace fs0 {  class ActionBase; }

namespace fs0 { namespace language { namespace fstrips {

class LogicalElement;
class Term;
class Formula;
class ActionEffect;
class FluentHeadedNestedTerm;


//! A number of helper methods to compute and deal with action / formula / term scopes
class ScopeUtils {
public:
	typedef std::set<const FluentHeadedNestedTerm*> TermSet;
	
	//! Computes the direct scope of a formula or term, i.e. a vector with all state variables involved in the term.
	static std::vector<VariableIdx> computeDirectScope(const LogicalElement* term);
	static void computeDirectScope(const LogicalElement* term, std::set<VariableIdx>& scope);
	
	//! Computes the direct scope of an action effect term
	static std::vector<VariableIdx> computeDirectScope(const ActionEffect* effect);
	static void computeDirectScope(const ActionEffect* effect, std::set<VariableIdx>& scope);
	
	//! Computes the indirect scope of an effect, i.e. the set of all the state variables involved in the effect that are produced by nested fluents
	static std::vector<const FluentHeadedNestedTerm*> computeIndirectScope(const ActionEffect* effect);
	static void computeIndirectScope(const ActionEffect* effect, TermSet& scope);
	
	//! Computes the indirect scope of a formula
	static void computeIndirectScope(const Formula* formula, ScopeUtils::TermSet& scope);
	
	//! Computes the full scope of a given formula, including state variables derived from nested fluents.
	static void computeFullScope(const Formula* formula, std::set<VariableIdx>& scope);
	
	//! Returns the direct scope of an action, i.e. the set of all the state variables that are directly relevant
	//! to either the preconditions or some effect of the action.
	//! This excludes indirect relevant actions produced by nested fluents.
	static std::vector<VariableIdx> computeActionDirectScope(const ActionBase& action);
	
	//! Computes the full scope of an action, which currently takes only into account variables appearing in the action precondition.
	static void computeActionFullScope(const ActionBase& action, std::set<VariableIdx>& scope);
	
	//!
	static void computeIndirectTermScope(const Term* term, std::set<VariableIdx>& scope);

	//! Returns the state variables (both direct and indirect) affected by the given action
	static void compute_affected(const ActionBase& action, std::set<VariableIdx>& scope);
	
	//!
	static std::vector<Atom> compute_affected_atoms(const ActionEffect* effect);
	
	//! Adds to 'variables' all those state variables that can be derived from the given element (formula / term),
	//! including variables which are directly present and those that are present through nested terms.
	//! It DOES NOT take into account "predicative" state variables, since these are not considered state variables anymore.
	static void computeRelevantElements(const LogicalElement* element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols);
};

} } } // namespaces
