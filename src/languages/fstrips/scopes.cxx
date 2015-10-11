
#include <problem.hxx>
#include <languages/fstrips/scopes.hxx>
#include <actions/ground_action.hxx>

namespace fs0 { namespace language { namespace fstrips {

std::vector<VariableIdx> ScopeUtils::computeDirectScope(Term::cptr term) {
	std::set<VariableIdx> set;
	computeDirectScope(term, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(Term::cptr term, std::set<VariableIdx>& scope) {
	for (Term::cptr subterm:term->flatten()) {
		if (auto sv = dynamic_cast<StateVariable::cptr>(subterm)) {
			scope.insert(sv->getValue());
		}
	}
}

std::vector<FluentHeadedNestedTerm::cptr> ScopeUtils::computeIndirectScope(ActionEffect::cptr effect) {
	TermSet set;
	computeIndirectScope(effect, set);
	return std::vector<FluentHeadedNestedTerm::cptr>(set.cbegin(), set.cend());
}

void ScopeUtils::computeIndirectScope(ActionEffect::cptr effect, TermSet& scope) {
	for (Term::cptr term:effect->flatten()) {
		if (term == effect->lhs()) continue; // Don't register the LHS root
		if (auto fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(term)) {
			scope.insert(fluent);
		}
	}
}

std::vector<VariableIdx> ScopeUtils::computeDirectScope(ActionEffect::cptr effect) {
	std::set<VariableIdx> set;
	computeDirectScope(effect, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(ActionEffect::cptr effect, std::set<VariableIdx>& scope) {
	// The left hand side of the effect only contributes to the set of relevant variables if it itself
	// is headed by a fluent function containing other state variables.
	if (FluentHeadedNestedTerm::cptr lhs_fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(effect->lhs())) {
		computeDirectScope(lhs_fluent, scope); // The direct scope of the nested fluent includes only the subterms' scope
	}
	computeDirectScope(effect->rhs(), scope);
}

std::vector<VariableIdx> ScopeUtils::computeDirectScope(AtomicFormula::cptr formula) {
	std::set<VariableIdx> set;
	computeDirectScope(formula, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(AtomicFormula::cptr formula, std::set<VariableIdx>& scope) {
	for (Term::cptr subterm:formula->getSubterms()) ScopeUtils::computeDirectScope(subterm, scope);
}

void ScopeUtils::computeIndirectScope(AtomicFormula::cptr formula, TermSet& scope) {
	for (Term::cptr term:formula->flatten()) {
		if (auto fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(term)) {
			scope.insert(fluent);
		}
	}
}

std::vector<VariableIdx> ScopeUtils::computeActionDirectScope(const GroundAction& action) {
	std::set<VariableIdx> unique;
	for (const AtomicFormula::cptr formula:action.getConditions()) computeDirectScope(formula, unique);
	return std::vector<VariableIdx>(unique.cbegin(), unique.cend());
}

void ScopeUtils::computeIndirectScope(FluentHeadedNestedTerm& nested, std::set<VariableIdx>& scope) {
	const ProblemInfo& info = Problem::getInfo();
	const std::vector<VariableIdx>& possible_variables = info.resolveStateVariable(nested.getSymbolId());
	scope.insert(possible_variables.cbegin(), possible_variables.cend());
}

} } } // namespaces
