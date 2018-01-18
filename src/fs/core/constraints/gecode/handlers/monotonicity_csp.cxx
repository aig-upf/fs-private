
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
#include <fs/core/constraints/gecode/extensions.hxx>


namespace fs0 { namespace gecode {
	
MonotonicityCSP::MonotonicityCSP(const fs::Formula* formula, const AtomIndex& tuple_index, const AllTransitionGraphsT& transitions, bool complete)
	:  FormulaCSP(formula, tuple_index, !complete),
       _monotonicity(tuple_index, transitions)
{}

GecodeCSP* MonotonicityCSP::build_root_csp(const State& root) const {
    assert(_monotonicity.is_active());

    // For the root node, we build an "artificial" changeset consisting of all
    // atoms in the state.
    std::vector<Atom> changeset;
    unsigned n = root.numAtoms();
    changeset.reserve(root.numAtoms());
    for (unsigned var = 0; var < n; ++var) {
        changeset.emplace_back(var, root.getValue(var));
    }

    return check_consistency_from_changeset(*_base_csp, root, changeset);
}

GecodeCSP* MonotonicityCSP::
instantiate_from_changeset(const GecodeCSP& parent_csp, const State& state, const std::vector<Atom>& changeset) const {
    if (_failed) return nullptr;

    // First update the domains of those CSP variables that correspond to state variables
    // which have their domain restricted by monotonicity constraints
    auto clone = static_cast<GecodeCSP*>(parent_csp.clone());
    _translator.updateStateVariableDomains(*clone, _monotonicity.compute_domains(changeset));

    // And then update the extensions of extensional constraints that might result from e.g.
    // nested fluents, etc., building the new extensions from the actual domains allowed to
    // each state variable of the problem. By definition of monotonicity, these extensions will
    // be subsets of the previous ones, so we can place them as new constraints
    const ProblemInfo& info = ProblemInfo::getInstance();

    std::unordered_set<unsigned> relevant_symbols;
    // TODO Move this iteration to preprocessing
    for (const ExtensionalConstraint& constraint:_extensional_constraints) {
        relevant_symbols.insert(constraint.get_term()->getSymbolId());
    }

    std::unordered_set<unsigned> relevant_symbols_changed;
    for (const auto& atom:changeset) {
        unsigned symbol = _tuple_index.var_to_symbol(atom.getVariable());

        if (relevant_symbols.find(symbol) != relevant_symbols.end()) {
            relevant_symbols_changed.insert(symbol);
        }
    }

    // relevant_symbols_changed contains all those symbol IDs whose denotation has been affected
    // in the last transition
    std::unordered_map<unsigned, Gecode::TupleSet> tuplesets;
    for (unsigned symbol:relevant_symbols) {
        auto& tupleset = tuplesets[symbol];


        for (VariableIdx var:info.getSymbolData(symbol).getVariables()) {
            const auto& value = state.getValue(var);

            // partial_extension is the extension of 'symbol' induced by
            // the state variable var
            const auto& partial_extension = _monotonicity.compute_partial_extension(var, value);
            for (const auto& tuple:partial_extension) {
                tupleset.add(tuple);
            }
        }

        tupleset.finalize();
    }


//    auto st = clone->status();
//    std::cout << "Status before posting extensional constraints: " << st << std::endl;

    for (const ExtensionalConstraint& constraint:_extensional_constraints) {
        unsigned symbol = constraint.get_term()->getSymbolId();
        if (relevant_symbols_changed.find(symbol) != relevant_symbols_changed.end()) {
            std::cout << "Ext. constraint: " << constraint << std::endl;
            if (!constraint.update(*clone, _translator, tuplesets.at(symbol))) {
                delete clone;
                return nullptr;
            }
        }
    }

//    st = clone->status();
//    std::cout << "Status after posting extensional constraints: " << st << std::endl;

    return clone;
}

bool MonotonicityCSP::
check_transitions(const State& parent, const State& child, const std::vector<Atom>& changeset) const {
    for (const auto &atom:changeset) {
        const auto& var = atom.getVariable();
        if (!_monotonicity.transition_is_valid(var, parent.getValue(var), atom.getValue())) {
//            LPT_DEBUG("cout", "Invalid transition in state: " << child)
            return false;
        }
    }
    return true;
}

GecodeCSP* MonotonicityCSP::
check(const GecodeCSP& parent_csp, const State& parent, const State& child, const std::vector<Atom>& changeset) const {
    // We first check that all transitions are valid
    if (!check_transitions(parent, child, changeset)) return nullptr;

    // Then check that the monotonicity CSP is consistent
    return check_consistency_from_changeset(parent_csp, child, changeset);

}

GecodeCSP* MonotonicityCSP::
check_consistency_from_changeset(const GecodeCSP& parent_csp, const State& child, const std::vector<Atom>& changeset) const {
    GecodeCSP* csp = instantiate_from_changeset(parent_csp, child, changeset);
    return check_consistency(csp);
}

GecodeCSP* MonotonicityCSP::
check_consistency(GecodeCSP* csp) const {

    if (!csp || !csp->checkConsistency()) { // This colaterally enforces propagation of constraints
        delete csp;
        return nullptr;
    }

//    std::cout << "\n(Locally consistent) monotonicity CSP: " << std::endl;
//    _translator.print(std::cout, *csp);

    if (!_approximate) {  // Solve the CSP completely
        GecodeCSP* solution = solve_csp(csp);
        if (!solution) return nullptr;
        // TODO Do something with the solution? Cache it?

//        std::cout << "\nComplete solution to monotonicity CSP: " << std::endl;
//        _translator.print(std::cout, *solution);
    }

    return csp;
}

GecodeCSP*
MonotonicityCSP::solve_csp(GecodeCSP* csp) {
    Gecode::DFS<GecodeCSP> engine(csp);
    return engine.next();
}

    } } // namespaces
