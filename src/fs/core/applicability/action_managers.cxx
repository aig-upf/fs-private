
#include <numeric>
#include <unordered_set>

#include <lapkt/tools/logging.hxx>

#include <fs/core/utils/system.hxx>

#include <fs/core/applicability/action_managers.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/state.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/utils/utils.hxx>

namespace fs0 {

NaiveApplicabilityManager::NaiveApplicabilityManager(const std::vector<const fs::Formula*>& state_constraints)
	: _state_constraints(state_constraints) {}

//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
bool NaiveApplicabilityManager::isApplicable(const State& state, const GroundAction& action, bool enforce_state_constraints) const {
    if (!action.isControl()) return false;
	if (!checkFormulaHolds(action.getPrecondition(), state)) return false;

    if (enforce_state_constraints && !_state_constraints.empty()) { // If we have no constraints, we can spare the cost of creating the new state.
		auto atoms = computeEffects(state, action);
        State next(state, atoms);
        for ( auto c : _state_constraints )
            if ( !checkFormulaHolds( c, next ) ) return false;
    }

	return true;
}

bool NaiveApplicabilityManager::isActive(const State& state, const GroundAction& action) const {
    if (!action.isNatural()) return false;
	if (!checkFormulaHolds(action.getPrecondition(), state)) return false;
	return true;
}

//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
std::vector<Atom>
NaiveApplicabilityManager::computeEffects(const State& state, const GroundAction& action) {
	std::vector<Atom> atoms;
    action.apply(state,atoms);
	return atoms;
}

void
NaiveApplicabilityManager::computeEffects(const State& state, const GroundAction& action, std::vector<Atom>& atoms) {
	const ProblemInfo& info = ProblemInfo::getInstance();
	const auto& effects = action.getEffects();
	atoms.clear();
	atoms.reserve(effects.size());
	for (const fs::ActionEffect* effect:effects) {
		if (effect->applicable(state)) {
			Atom atom = effect->apply(state);
			// A safety check - perhaps introduces some overhead
			info.checkValueIsValid(atom.getVariable(), atom.getValue());
			atoms.push_back(std::move(atom));
		}
	}
}

bool NaiveApplicabilityManager::checkFormulaHolds(const fs::Formula* formula, const State& state) {
	return formula->interpret(state);
}

bool NaiveApplicabilityManager::checkStateConstraints(const State& state) const {
    for ( auto c : _state_constraints )
        if (!NaiveApplicabilityManager::checkFormulaHolds(c, state)) return false;
    return true;
}


//! A small helper
object_id _extract_constant_val(const fs::Term* lhs, const fs::Term* rhs) {
	const fs::Constant* _lhs = dynamic_cast<const fs::Constant*>(lhs);
	const fs::Constant* _rhs = dynamic_cast<const fs::Constant*>(rhs);
	assert(_lhs || _rhs);
	return _lhs ? _lhs->getValue() : _rhs->getValue();
}


void
BasicApplicabilityAnalyzer::build(bool build_applicable_index) {
	LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [0]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");

	const ProblemInfo& info = ProblemInfo::getInstance();

	if (build_applicable_index) {
		_applicable.resize(_tuple_idx.size());
	}
	_rev_applicable.resize(_actions.size());
	_variable_relevance = std::vector<unsigned>(info.getNumVariables(), 0);

	LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() - TupleIdx size: " << _tuple_idx.size());
	LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() - Actions size: " << _actions.size());
	LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() - size of set of atomidx: " << sizeof(std::unordered_set<AtomIdx>));
	LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [1]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");

	for (unsigned i = 0; i < _actions.size(); ++i) {

		/* DEBUGGING

		if (i%100==0) {
			LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [it. " << i << "]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
			unsigned cnt = 0;
			for (auto& app_set:_applicable) cnt += app_set.size();
			LPT_DEBUG("cout", "Aggregated '_applicable' size [it. " << i << "]: " << cnt);

			cnt = 0;
			for (auto& app_set:_rev_applicable) cnt += app_set.size();
			LPT_DEBUG("cout", "Aggregated '_rev_applicable' size [it. " << i << "]: " << cnt);
		}
		*/
		const GroundAction& action = *_actions[i];
		if (dynamic_cast<const fs::Tautology*>(action.getPrecondition())) { // If there's no precondition, the action is always potentially applicable
			for (auto& app_set:_applicable) app_set.push_back(i);
			continue;
		}

		const fs::Conjunction* precondition = dynamic_cast<const fs::Conjunction*>(action.getPrecondition());
		if (!precondition) { // If the precondition is not a conjunction, we cannot say much, so we consider the action as always potentially applicable
			for (auto& app_set:_applicable) app_set.push_back(i);
			continue;
		}

		auto preconditions = fs::check_all_atomic_formulas(precondition->getSubformulae());


		std::set<VariableIdx> referenced; // The state variables already made reference to by some precondition
		for (const fs::AtomicFormula* conjunct:preconditions) {
			const auto* rel = dynamic_cast<const fs::RelationalFormula*>(conjunct);
			const auto* eq = dynamic_cast<const fs::EQAtomicFormula*>(conjunct);
			const auto* neq = dynamic_cast<const fs::NEQAtomicFormula*>(conjunct);
			unsigned nestedness = fs::nestedness(*conjunct);
			std::vector<VariableIdx> all_relevant = fs::ScopeUtils::computeDirectScope(conjunct);

			// This implements a very rudimentary test that indexes only preconditions of the form X = x or X != x,
			// furthermore assuming that there are no two preconditions making reference to the same state variable
			if (nestedness > 0 || all_relevant.size() != 1 || !(eq || neq)) continue;

			const auto* sv = dynamic_cast<const fs::StateVariable*>(rel->lhs());
			if (!sv) continue;

// 			std::cout << "Processing action #" << i << ": " << action << std::endl;
// 			std::cout << "Processing conjunct: " << *conjunct << std::endl;

			VariableIdx relevant = all_relevant[0];
//			if (!referenced.insert(relevant).second) {
//				LPT_DEBUG("cout", "Conjunct \"" << *conjunct << "\" contains a duplicate reference to state variable \"" << info.getVariableName(relevant) << "\"");
//				throw std::runtime_error("BasicApplicabilityAnalyzer requires that no two preconditions make reference to the same state variable");
//			}

			_variable_relevance[relevant]++;

			if (eq) { // Prec is of the form X=x
// 				std::cout << "Precondition: " << *eq << std::endl;
				object_id value = _extract_constant_val(eq->lhs(), eq->rhs());
				AtomIdx tup = _tuple_idx.to_index(relevant, value);

// 				std::cout << "Corresponding Atom: " << _tuple_idx.to_atom(tup) << std::endl;

				if (build_applicable_index) {
					_applicable[tup].push_back(i);
				}
				_rev_applicable[i].insert(tup);

			} else { // Prec is of the form X!=x
				assert(neq);
// 				std::cout << "Precondition: " << *neq << std::endl;
				object_id value = _extract_constant_val(neq->lhs(), neq->rhs());
				const std::vector<object_id>& values = info.getVariableObjects(relevant);
				for (object_id v2:values) {
					if (v2 != value) {
						AtomIdx tup = _tuple_idx.to_index(relevant, v2);
						if (build_applicable_index) {
							_applicable[tup].push_back(i);
						}
						_rev_applicable[i].insert(tup);
					}
				}
			}
		}

		// Now, for those state variables that have _not_ been referenced, the action is potentially applicable no matter what value the state variable takes.
		if (build_applicable_index) {
			for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
				if (referenced.find(var) != referenced.end()) continue;

				for (object_id val:info.getVariableObjects(var)) {
					AtomIdx tup = _tuple_idx.to_index(var, val);
					_applicable[tup].push_back(i);
				}
			}
		}
	}

	_total_actions =  _actions.size();
	LPT_DEBUG("cout", "Mem. usage in BasicApplicabilityAnalyzer::build() [END]: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
}


//! A local helper to build a list <0,1,...,size>
std::vector<ActionIdx> _build_all_actions_whitelist(unsigned size) {
	std::vector<ActionIdx> vector(size);
	std::iota(vector.begin(), vector.end(), 0);
	return vector;
}

//! A local helper to ensure the provided state constraints are of the adequate form.
std::vector<const fs::Formula*>
_process_state_constraints(const fs::Formula* state_constraints) {
	if (dynamic_cast<const fs::Tautology*>(state_constraints)) return {};

	const fs::Conjunction* conjunction = dynamic_cast<const fs::Conjunction*>(state_constraints);
	if (!conjunction) throw std::runtime_error("Unsupported State Constraint type");

	return conjunction->getSubformulae();
}


NaiveActionManager::NaiveActionManager(const std::vector<const GroundAction*>& actions, const std::vector<const fs::Formula*>& state_constraints) :
	_actions(actions),
	_state_constraints(state_constraints),
	_all_actions_whitelist(_build_all_actions_whitelist(actions.size()))
{}

bool
NaiveActionManager::applicable(const State& state, const GroundAction& action, bool enforce_state_constraints) const {
    if (!action.isControl()) return false;
	if (!NaiveApplicabilityManager::checkFormulaHolds(action.getPrecondition(), state)) return false;

	if (enforce_state_constraints && !_state_constraints.empty()) { // If we have no constraints, we can spare the cost of further checks
		NaiveApplicabilityManager::computeEffects(state, action, _effects_cache);
		State next(state, _effects_cache);
		return check_constraints(action.getId(), next);
	}
	return true;
}

GroundApplicableSet
NaiveActionManager::applicable(const State& state, bool enforce_state_constraints) const {
	return GroundApplicableSet(*this, state, compute_whitelist(state),enforce_state_constraints);
}


bool
NaiveActionManager::check_constraints(unsigned applied_action_id, const State& state) const {
	for (const auto constraint:_state_constraints) { // Check all state constraints
		if (!constraint->interpret(state)) return false;
	}
	return true;
}

} // namespaces
