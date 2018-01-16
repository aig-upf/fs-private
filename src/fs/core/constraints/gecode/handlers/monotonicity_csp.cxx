
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/constraints/gecode/handlers/monotonicity_csp.hxx>
#include <fs/core/constraints/gecode/helper.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/constraints/gecode/utils/novelty_constraints.hxx>
#include <fs/core/constraints/gecode/supports.hxx>
#include <fs/core/state.hxx>

#include <lapkt/tools/logging.hxx>


namespace fs0 { namespace gecode {
	
MonotonicityCSP::MonotonicityCSP(const fs::Formula* formula, const AtomIndex& tuple_index, const AllTransitionGraphsT& transitions, bool approximate)
	:  FormulaCSP(formula, tuple_index, approximate),
       _monotonicity(transitions)
{}

GecodeCSP* MonotonicityCSP::
instantiate_from_changeset(const GecodeCSP& parent_csp, const State& state, const std::vector<Atom>& changeset) const {
    if (_failed) return nullptr;

    auto clone = static_cast<GecodeCSP*>(parent_csp.clone());

    const auto allowed_domains = _monotonicity.compute_domains(changeset);
    _translator.updateStateVariableDomains(*clone, allowed_domains);

    // TODO Is this really necessary??
    for (const ExtensionalConstraint& constraint:_extensional_constraints) {
        if (!constraint.update(*clone, _translator, state)) {
            delete clone;
            return nullptr;
        }
    }

    return clone;
}

GecodeCSP* MonotonicityCSP::
check(const GecodeCSP& parent_csp, const State* parent, const State& child, const std::vector<Atom>& changeset) const {
    // We first check that all transitions are valid, if there have been indeed
    // transitions (i.e. if there is a parent state)
    if (parent) {
        for (const auto &atom:changeset) {
            const auto& var = atom.getVariable();
            if (!_monotonicity.transition_is_valid(var, parent->getValue(var), atom.getValue())) return nullptr;
        }
    }

    // Then check that the monotonicity CSP is consistent
    return check_consistency(parent_csp, child, changeset);

}

GecodeCSP* MonotonicityCSP::
check_consistency(const GecodeCSP& parent_csp, const State& child, const std::vector<Atom>& changeset) const {
    GecodeCSP* csp = instantiate_from_changeset(parent_csp, child, changeset);

    if (!csp || !csp->checkConsistency()) { // This colaterally enforces propagation of constraints
        delete csp;
        return nullptr;
    }

    if (!_approximate) {  // Solve the CSP completely
//        GecodeCSP* solution = solve_csp(csp);
        // TODO Do something with the solution
        assert(0);
    }

    return csp;
}

GecodeCSP*
MonotonicityCSP::solve_csp(GecodeCSP* csp) {
    Gecode::DFS<GecodeCSP> engine(csp);
    GecodeCSP* solution = engine.next();
    return solution;
}

} } // namespaces
