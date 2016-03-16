
#include <problem.hxx>
#include <languages/fstrips/scopes.hxx>
#include <actions/ground_action.hxx>
#include <boost/concept_check.hpp>

namespace fs0 { namespace language { namespace fstrips {

std::vector<VariableIdx> ScopeUtils::computeDirectScope(Term::cptr term) {
	std::set<VariableIdx> set;
	computeDirectScope(term, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(Term::cptr term, std::set<VariableIdx>& scope) {
	for (Term::cptr subterm:term->all_terms()) {
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
	for (Term::cptr term:effect->all_terms()) {
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

std::vector<VariableIdx> ScopeUtils::computeDirectScope(Formula::cptr formula) {
	std::set<VariableIdx> set;
	computeDirectScope(formula, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(Formula::cptr formula, std::set<VariableIdx>& scope) {
	for (Term::cptr subterm:formula->all_terms()) ScopeUtils::computeDirectScope(subterm, scope);
}

void ScopeUtils::computeIndirectScope(Formula::cptr formula, TermSet& scope) {
	for (Term::cptr term:formula->all_terms()) {
		if (auto fluent = dynamic_cast<FluentHeadedNestedTerm::cptr>(term)) {
			scope.insert(fluent);
		}
	}
}

std::vector<VariableIdx> ScopeUtils::computeActionDirectScope(const BaseAction& action) {
	return computeDirectScope(action.getPrecondition());
}

void ScopeUtils::computeIndirectScope(FluentHeadedNestedTerm& nested, std::set<VariableIdx>& scope) {
	const ProblemInfo& info = Problem::getInfo();
	const std::vector<VariableIdx>& possible_variables = info.resolveStateVariable(nested.getSymbolId());
	scope.insert(possible_variables.cbegin(), possible_variables.cend());
}

std::vector<Atom> ScopeUtils::compute_affected_atoms(ActionEffect::cptr effect) {
	if (dynamic_cast<FluentHeadedNestedTerm::cptr>(effect->lhs())) {
		throw std::runtime_error("Nested LHS terms not yet supported");
	}
	auto statevar = dynamic_cast<StateVariable::cptr>(effect->lhs());
	assert(statevar);
	
	VariableIdx variable = statevar->getValue();
	
	if (Constant::cptr rhs_const = dynamic_cast<Constant::cptr>(effect->rhs())) {
		return { Atom(variable, rhs_const->getValue()) };
	}
	
	// Otherwise, ATM we simply overapproximate the set of all potentially affected atoms by considering
	// that all values for that variable (consistent with the RHS type) can be achieved.
	const ProblemInfo& info = Problem::getInfo();
	std::vector<Atom> affected;
	TypeIdx type = effect->rhs()->getType();
	for (auto value:info.getTypeObjects(type)) {
		affected.push_back(Atom(variable, value));
	}
	return affected;
}


} } } // namespaces
