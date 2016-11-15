
#include <unordered_set>

#include <aptk2/tools/logging.hxx>

#include <state.hxx>
#include <search/drivers/bfws/ctmp_action_manager.hxx>
#include <languages/fstrips/language.hxx>



namespace fs0 {


bool
CTMPActionManager::check_constraints(unsigned action_id, const State& state) const {
	// Check only those constraints that can be affected by the action last applied
	for (const fs::AtomicFormula* constraint:_sc_index[action_id]) {
		if (!constraint->interpret(state)) return false;
	}
	return true;
}

} // namespaces


