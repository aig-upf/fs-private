

#include <lapkt/tools/logging.hxx>
#include <applicability/gecode_analyzer.hxx>
#include <problem_info.hxx>
#include <constraints/gecode/handlers/ground_action_csp.hxx>
#include <utils/atom_index.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>

namespace fs0 {

using namespace gecode;

void
GecodeApplicabilityAnalyzer::build(bool build_applicable_index) {
	const ProblemInfo& info = ProblemInfo::getInstance();

	LPT_DEBUG("cout", "Analyzing applicability of actions...");

	_applicable.resize(_tuple_idx.size());
// 	std::vector<std::vector<ActionIdx>> index(_tuple_idx.size());
// 	return index;

	//! We'll store here the actions that are potentially applicable to at least one atom
	std::unordered_set<unsigned> potentially_applicable;

	for (unsigned i = 0; i < _actions.size(); ++i) {
		const GroundAction& action = *_actions[i];

		LPT_DEBUG("cout", "Analyzing action " << action);

		// If there's no precondition, the action is always potentially applicable
		if (dynamic_cast<const fs::Tautology*>(action.getPrecondition())) {
			for (auto& app_set:_applicable) app_set.push_back(i);
			LPT_DEBUG("cout", "\tAction has no precondition and is thus ever-applicable");
			potentially_applicable.insert(i);
			continue;
		}

		// Build a CSP from the action
		GroundActionCSP manager(action, _tuple_idx, false, false);
		if (!manager.init(false)) {
			LPT_DEBUG("cout", "\tAction considered inapplicable");
			continue;
		}


		std::set<VariableIdx> all_relevant;
		fs::ScopeUtils::computeActionFullScope(action, all_relevant);

		// For each variable X relevant to the action precondition, and for each possible value x it might have,
		// we check whether there is at least one solution to the CSP where X=x.
		// (kind of a local consistency test _as if_ the whole precondition was a sole constraint)
		// If that is the case, we consider the action potentially applicable when X=x
		for (VariableIdx relevant:all_relevant) {
			for (const object_id&
 value:info.getVariableObjects(relevant)) {

				GecodeCSP* restricted = manager.post(relevant, value);
				if (manager.check_one_solution_exists(restricted)) {
					AtomIdx tup = _tuple_idx.to_index(relevant, value);
					_applicable[tup].push_back(i);
					potentially_applicable.insert(i);
				}
			}
		}

		// Now, for those state variables that have _not_ been referenced, the action is potentially applicable no matter what value the state variable takes.
		for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
			if (all_relevant.find(var) != all_relevant.end()) continue;

			for (const object_id&
 val:info.getVariableObjects(var)) {
				AtomIdx tup = _tuple_idx.to_index(var, val);
				_applicable[tup].push_back(i);
				potentially_applicable.insert(i);
			}
		}
	}

	_total_actions = potentially_applicable.size();
}


} // namespaces
