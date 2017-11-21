
#include <fs/core/problem_info.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>

#include <fs/core/actions/actions.hxx>
#include <fs/core/constraints/gecode/utils/nested_fluent_iterator.hxx>
#include <fs/core/utils/utils.hxx>

namespace fs0 { namespace language { namespace fstrips {

std::vector<VariableIdx> ScopeUtils::computeDirectScope(const LogicalElement* element) {
	std::set<VariableIdx> set;
	computeDirectScope(element, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(const LogicalElement* element, std::set<VariableIdx>& scope) {
	auto state_variables = Utils::filter_by_type<const StateVariable*>(all_nodes(*element));
	for (const StateVariable* sv:state_variables) {
		scope.insert(sv->getValue());
	}
}


std::vector<const FluentHeadedNestedTerm*> ScopeUtils::computeIndirectScope(const ActionEffect* effect) {
	TermSet set;
	computeIndirectScope(effect, set);
	return std::vector<const FluentHeadedNestedTerm*>(set.cbegin(), set.cend());
}

void ScopeUtils::computeIndirectScope(const ActionEffect* effect, TermSet& scope) {
	for (const Term* term:effect->all_terms()) {
		if (term == effect->lhs()) continue; // Don't register the LHS root
		if (auto fluent = dynamic_cast<const FluentHeadedNestedTerm*>(term)) {
			scope.insert(fluent);
		}
	}
}

std::vector<VariableIdx> ScopeUtils::computeDirectScope(const ActionEffect* effect) {
	std::set<VariableIdx> set;
	computeDirectScope(effect, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(const ActionEffect* effect, std::set<VariableIdx>& scope) {
	// The left hand side of the effect only contributes to the set of relevant variables if it itself
	// is headed by a fluent function containing other state variables.
	if (const FluentHeadedNestedTerm* lhs_fluent = dynamic_cast<const FluentHeadedNestedTerm*>(effect->lhs())) {
		computeDirectScope(lhs_fluent, scope); // The direct scope of the nested fluent includes only the subterms' scope
	}
	computeDirectScope(effect->rhs(), scope);
}


void ScopeUtils::computeIndirectScope(const Formula* formula, TermSet& scope) {
	for (const Term* term:fs::all_terms(*formula)) {
		if (auto fluent = dynamic_cast<const FluentHeadedNestedTerm*>(term)) {
			scope.insert(fluent);
		}
	}
}

std::vector<VariableIdx> ScopeUtils::computeActionDirectScope(const ActionBase& action) {
	return computeDirectScope(action.getPrecondition());
}


void _computeRelevantElements(const std::vector<const Term*>& all_terms, std::set<VariableIdx>& variables, std::set<unsigned>& symbols, bool preds_as_state_vars=false) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (const Term* term:all_terms) {
		auto statevar = dynamic_cast<const StateVariable*>(term);
		auto fluent = dynamic_cast<const FluentHeadedNestedTerm*>(term);
		
		if (!statevar && !fluent) continue;
		
		if (statevar) {
			unsigned symbol = statevar->getOrigin()->getSymbolId();
			
			if (info.isPredicate(symbol)) {
				symbols.insert(symbol);
				if (preds_as_state_vars) {
					variables.insert(statevar->getValue());
				}
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


void ScopeUtils::computeRelevantElements(const LogicalElement* element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols) {
	_computeRelevantElements(fs::all_terms(*element), variables, symbols);
}

void ScopeUtils::computeFullScope(const Formula* formula, std::set<VariableIdx>& scope) {
	std::set<unsigned> _;
	_computeRelevantElements(fs::all_terms(*formula), scope, _, true);
}

void ScopeUtils::computeActionFullScope(const ActionBase& action, std::set<VariableIdx>& scope) {
	std::set<unsigned> _;
	_computeRelevantElements(fs::all_terms(*action.getPrecondition()), scope, _, true);
}

void ScopeUtils::compute_affected(const ActionBase& action, std::set<VariableIdx>& scope) {
	std::set<unsigned> _;
	for (const fs::ActionEffect* eff:action.getEffects()) {
		_computeRelevantElements(fs::all_terms(*eff->lhs()), scope, _, true);
	}
}

} } } // namespaces
