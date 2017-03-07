
#include <problem_info.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/operations.hxx>

#include <actions/actions.hxx>
#include <constraints/gecode/utils/nested_fluent_iterator.hxx>

namespace fs0 { namespace language { namespace fstrips {

std::vector<VariableIdx> ScopeUtils::computeDirectScope(const Term* term) {
	std::set<VariableIdx> set;
	computeDirectScope(term, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(const Term* term, std::set<VariableIdx>& scope) {
	for (const Term* subterm:term->all_terms()) {
		if (auto sv = dynamic_cast<const StateVariable*>(subterm)) {
			scope.insert(sv->getValue());
		}
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

std::vector<VariableIdx> ScopeUtils::computeDirectScope(const Formula* formula) {
	std::set<VariableIdx> set;
	computeDirectScope(formula, set);
	return std::vector<VariableIdx>(set.cbegin(), set.cend());
}

void ScopeUtils::computeDirectScope(const Formula* formula, std::set<VariableIdx>& scope) {
	for (const Term* subterm:fs::all_terms(*formula)) ScopeUtils::computeDirectScope(subterm, scope);
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

std::vector<Atom> ScopeUtils::compute_affected_atoms(const ActionEffect* effect) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::vector<VariableIdx> lhs_variables;
	std::vector<Atom> affected;
	
	if (auto statevar = dynamic_cast<const StateVariable*>(effect->lhs())) {
		lhs_variables.push_back(statevar->getValue());
	} else if (auto nested = dynamic_cast<const FluentHeadedNestedTerm*>(effect->lhs())) {
		lhs_variables = info.resolveStateVariable(nested->getSymbolId()); // TODO - This is a gross overapproximation
	} else throw std::runtime_error("Unsupported effect type");
	
	// Now gather, for each possible variables, all possible values
	for (VariableIdx variable:lhs_variables) {
		const Constant* rhs_const = dynamic_cast<const Constant*>(effect->rhs());
		
		// TODO - All this should be greatly simplified when we have a proper distinction between functional effects
		// and predicative add/del effects
		if (info.isPredicativeVariable(variable)){
			if (!rhs_const) throw std::runtime_error("A predicative effect cannot be assigned anything other than a constant");
			if (rhs_const->getValue() == 1) { // Push only non-negated effects
				affected.push_back(Atom(variable, 1));
			}
			continue;
		}
		
		// If we reached this point we surely have a non-predicative effect.
		if (rhs_const) {
			affected.push_back(Atom(variable, rhs_const->getValue()));
			
		} else {
		
			// Otherwise, ATM we simply overapproximate the set of all potentially affected atoms by considering
			// that all values for that variable (consistent with the RHS type) can be achieved.
			const ProblemInfo& info = ProblemInfo::getInstance();
			
		// 	TypeIdx type = effect->rhs()->getType(); // This doesn't work for RHS such as X + 1
			TypeIdx type = fs::type(*effect->lhs());
			for (auto value:info.getTypeObjects(type)) {
				affected.push_back(Atom(variable, value));
			}
		}
	}
	return affected;
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


void ScopeUtils::computeRelevantElements(const Term* element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols) {
	_computeRelevantElements(element->all_terms(), variables, symbols);
}

void ScopeUtils::computeRelevantElements(const Formula* element, std::set<VariableIdx>& variables, std::set<unsigned>& symbols) {
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
		_computeRelevantElements(eff->lhs()->all_terms(), scope, _, true);
	}
}

} } } // namespaces
