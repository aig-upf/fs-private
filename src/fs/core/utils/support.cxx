
#include <unordered_set>

#include <fs/core/utils/support.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>


namespace fs0 { namespace support {


std::vector<bool> compute_managed_symbols(const std::vector<const ActionBase*>& actions, const fs::Formula* goal_formula, const std::vector<const fs::Formula*>& state_constraints) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	std::unordered_set<unsigned> symbols_in_nested_fluents;
	std::vector<bool> managed(info.getNumLogicalSymbols(), false); // To begin with, no symbol needs to be managed

	// We need to keep track of extensions for:
	// A) Fluent predicate symbols
	// B) Fluent function symbols appearing in nested expressions

	for (const ActionBase* action:actions) {
		for (const fs::ActionEffect* effect:action->getEffects()) {
			const fs::Term* lhs = effect->lhs();

			// Predicate symbols appearing in LHS state variables need to be tracked
			if (const fs::StateVariable* statevar = dynamic_cast<const fs::StateVariable*>(lhs)) {
				unsigned symbol = statevar->getSymbolId();
				if (info.isPredicate(symbol)) { // We have a fluent predicate symbol
					managed.at(symbol) = true;
				}
			}

			// Now: any symbol appearing in a nested fluent needs to be tracked
			for (const fs::FluentHeadedNestedTerm* fluent:Utils::filter_by_type<const fs::FluentHeadedNestedTerm*>(effect->all_terms())) {
				managed.at(fluent->getSymbolId()) = true;
			}
		}
	}

	for (const fs::FluentHeadedNestedTerm* fluent:Utils::filter_by_type<const fs::FluentHeadedNestedTerm*>(fs::all_terms(*goal_formula))) {
		managed.at(fluent->getSymbolId()) = true;
	}

	for ( auto sc : state_constraints )
		for (const fs::FluentHeadedNestedTerm* fluent:Utils::filter_by_type<const fs::FluentHeadedNestedTerm*>(fs::all_terms(*sc))) {
			managed.at(fluent->getSymbolId()) = true;
		}

	return managed;
}


} } // namespaces
