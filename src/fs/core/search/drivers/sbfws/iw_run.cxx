
#include "iw_run.hxx"

namespace fs0::bfws {

//! A helper to derive the distinct goal atoms
ConditionCounter
build_condition_counter(const fs::Formula* formula) {
    const auto* atom = dynamic_cast<const fs::EQAtomicFormula*>(formula);
    const auto* conjunction = dynamic_cast<const fs::Conjunction*>(formula);
    const auto* taut = dynamic_cast<const fs::Tautology*>(formula);
    if (!atom && !conjunction && !taut) {
        throw std::runtime_error("Cannot decompose action precondition into conditions");
    }

    std::vector<const fs::Formula*> conditions;

    if (taut) {
        // Do nothing
    } else if (atom) {
        conditions.push_back(atom);
    } else {
        for (const fs::Formula* a:conjunction->getSubformulae()) {
            conditions.push_back(a);
        }
    }

    return ConditionCounter(conditions);
}

} // namespaces
