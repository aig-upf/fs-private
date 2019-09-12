
#include <fs/core/languages/fstrips/language.hxx>

//! A helper to derive the distinct goal atoms
std::vector<const fs::Formula*>
obtain_goal_atoms(const fs::Formula* goal) {
    std::vector<const fs::Formula*> goal_atoms;

    const auto* conjunction = dynamic_cast<const fs::Conjunction*>(goal);
    if (!conjunction) {
        goal_atoms.push_back(goal);
    } else {
        for (const fs::Formula* atom:conjunction->getSubformulae()) {
            goal_atoms.push_back(atom);
        }
    }

    return goal_atoms;
}