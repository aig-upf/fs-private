
#pragma once
#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>
#include <constraints/gecode/utils/nested_fluent_iterator.hxx>


namespace fs0 {  class BaseAction; }

namespace fs0 { namespace language { namespace fstrips {

//! A number of helper methods to compute and deal with action / formula / term scopes
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
	static std::vector<VariableIdx> computeDirectScope(Formula::cptr formula);
	static void computeDirectScope(Formula::cptr formula, std::set< fs0::VariableIdx >& scope);
	
	//! Computes the indirect scope of a formula
	static void computeIndirectScope(Formula::cptr formula, ScopeUtils::TermSet& scope);
	
	//! Returns the direct scope of an action, i.e. the set of all the state variables that are directly relevant
	//! to either the preconditions or some effect of the action.
	//! This excludes indirect relevant actions produced by nested fluents.
	static std::vector<VariableIdx> computeActionDirectScope(const BaseAction& action);
	
	//! Computes all the (indirect) state variables in which a nested fluent might result
	// TODO - Check if really necessary
	static void computeIndirectScope(FluentHeadedNestedTerm& nested, std::set<VariableIdx>& scope);

	//!
	static void computeIndirectTermScope(Term::cptr term, std::set<VariableIdx>& scope);

	//!
	static std::vector<VariableIdx> compute_rhs_complete_scope(ActionEffect::cptr effect);
	static void compute_rhs_complete_scope(ActionEffect::cptr effect, std::set<VariableIdx>& scope);
	
	//! Adds to 'variables' all those state variables that can be derived from the given element (formula / term),
	//! including variables which are directly present and those that are present through nested terms.
	//! It DOES NOT take into account "predicative" state variables, since these are not considered state variables anymore.
	template <typename T>
	static void computeRelevantElements(const T& element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols) {
		const ProblemInfo& info = Problem::getInfo();
		for (Term::cptr term:element->all_terms()) {
			auto statevar = dynamic_cast<StateVariable::cptr>(term);
			auto fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(term);
			
			if (!statevar && !fluent) continue;
			
			if (statevar) {
				unsigned symbol = statevar->getOrigin()->getSymbolId();
				
				if (info.isPredicate(symbol)) {
					symbols.insert(symbol);
				} else {
					variables.insert(statevar->getValue());
				}
			}
			
			if (fluent) {
				unsigned symbol = fluent->getSymbolId();
				
				if (info.isPredicate(symbol)) {
					symbols.insert(symbol);
				} else {
					for (gecode::nested_fluent_iterator it(fluent); !it.ended(); ++it) {
						VariableIdx variable = it.getDerivedStateVariable();
						variables.insert(variable);
					}
				}
			}
		}
	}

};

} } } // namespaces
