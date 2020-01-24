
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
#include <fs/core/problem.hxx>
#include <fs/core/utils/config.hxx>

#include <lapkt/tools/logging.hxx>
#include <fs/core/constraints/gecode/extensions.hxx>



namespace fs0 { namespace gecode {

// Helper - generate a changeset reflecting all atoms in a state
std::vector<Atom>
_as_atoms(const State& state) {
    std::vector<Atom> changeset;
    unsigned n = state.numAtoms();
    changeset.reserve(state.numAtoms());
    for (unsigned var = 0; var < n; ++var) {
        changeset.emplace_back(var, state.getValue(var));
    }
    return changeset;
}


MonotonicityCSP::MonotonicityCSP(const fs::Formula* formula, const AtomIndex& tuple_index, const AllTransitionGraphsT& transitions, bool complete)
	:  FormulaCSP(formula, tuple_index, !complete),
       _monotonicity(tuple_index, transitions)
{}

GecodeSpace* MonotonicityCSP::
instantiate_from_changeset(const DomainTracker& base_domains, const State& state) const {

    // First update the domains of those CSP variables that correspond to state variables
    // which have their domain restricted by monotonicity constraints
    auto clone = static_cast<GecodeSpace*>(_gecode_space->clone());

    _translator.updateStateVariableDomains(*clone, base_domains.to_intsets(), true);

//    std::cout << "Num extensional constraints: " << _extensional_constraints.size() << std::endl;


    // And then update the extensions of extensional constraints that might result from e.g.
    // nested fluents, etc., building the new extensions from the actual domains allowed to
    // each state variable of the problem.
    const ProblemInfo& info = ProblemInfo::getInstance();

    // TODO Move this iteration to preprocessing
    std::unordered_set<unsigned> relevant_symbols;
    for (const ExtensionalConstraint& constraint:_extensional_constraints) {
//        std::cout << "Ext. constraint: " << constraint << std::endl;
        relevant_symbols.insert(constraint.get_term()->getSymbolId());
    }

//    assert(_extensional_constraints.empty()); // TODO Reimplement the code below
//    return clone;

    std::unordered_map<unsigned, Gecode::TupleSet> tuplesets;
    for (unsigned symbol:relevant_symbols) {
        auto& tupleset = tuplesets[symbol];


        for (VariableIdx var:info.getSymbolData(symbol).getVariables()) {
            const auto& value = state.getValue(var);

            // partial_extension is the subset of the extension of 'symbol' induced by 'var'
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
        if (relevant_symbols.find(symbol) != relevant_symbols.end()) {
//            std::cout << "Ext. constraint: " << constraint << std::endl;
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
check_transitions(const State& parent, const std::vector<Atom>& changeset) const {
    for (const auto &atom:changeset) {
        const auto& var = atom.getVariable();
        if (!_monotonicity.transition_is_valid(var, parent.getValue(var), atom.getValue())) {
//            LPT_DEBUG("cout", "Invalid transition in state: " << child)
            return false;
        }
    }
    return true;
}


DomainTracker MonotonicityCSP::
compute_base_domains(const DomainTracker& parent_domains,
                     const std::vector<Atom>& changeset) const {

    // Start with a fresh copy of the parent domains. We assume that there is no empty domain there.
    std::vector<TransitionGraph::BitmapT> resulting_domains(parent_domains.domains());

    // Compute reachable domains given by current state, only for those variables which have
    // changed their value wrt the parent state. If any of those variables is not monotonic,
    // a nullptr will be returned
    auto modified_variable_domains = _monotonicity.retrieve_domains(changeset);

//    std::cout << "Base: " << print::domain_tracker(DomainTracker(std::vector<TransitionGraph::BitmapT>(resulting_domains))) << std::endl;
//    std::cout << "Changeset: " << std::endl;

    // Compute the intersection (in place)
    assert(modified_variable_domains.size() == changeset.size());
    for (unsigned i = 0, s = changeset.size(); i < s; ++i) {
        VariableIdx var = changeset[i].getVariable();
        if (!_monotonicity.is_monotonic(var)) continue;

//        std::cout << print::bitset(var, modified_variable_domains[i]) << std::endl;
        assert(resulting_domains[var].size() == modified_variable_domains[i].size());
        resulting_domains[var] &= modified_variable_domains[i];
        if (resulting_domains[var].none()) return {};
    }

    return DomainTracker(std::move(resulting_domains));
}

DomainTracker MonotonicityCSP::
compute_root_domains(const State& root) const {

    // Compute reachable domains for all variables in the state (i.e., that are monotonic)
    // If any of those variables is not monotonic, an empty domain will be returned in their slot.
    auto domains = _monotonicity.retrieve_domains(_as_atoms(root));
    assert(domains.size() == root.numAtoms());

    return DomainTracker(std::move(domains));
}

DomainTracker MonotonicityCSP::
create_root(const State& root) const {
    DomainTracker tracker = compute_root_domains(root);
    return post_monotonicity_csp_from_domains(root, tracker, true);
}

DomainTracker MonotonicityCSP::
generate_node(const State& parent, const DomainTracker& parent_domains,
              const State& child, const std::vector<Atom>& changeset) const {
    assert(!parent_domains.is_null());

    // Check that all transitions are valid
    if (!check_transitions(parent, changeset)) return {};

    // Compute the domains that will form the basis for the CSP
    auto base_domains = compute_base_domains(parent_domains, changeset);

    return post_monotonicity_csp_from_domains(child, base_domains, false);
}

DomainTracker MonotonicityCSP::
post_monotonicity_csp_from_domains(const State& state, const DomainTracker& domains, bool stick_to_solution) const {
    // Check that there's no domain among the base, unpruned domains has already become empty
    // If that's the case, we can prune the node without even generating the CSP
    if (domains.is_null()) return {};

    // Otherwise, we generate the CSP and propagate constraints until reaching local consistency
    auto csp = instantiate_from_changeset(domains, state);
    csp = check_consistency(csp, stick_to_solution);
    if (!csp) return {};

    // If the CSP is consistent, we inspect the domains it assigns to variables, and delete it
    auto res = prune_domains(*csp, domains);
    delete csp;
    return res;
}





DomainTracker MonotonicityCSP::prune_domains(const GecodeSpace& csp, const DomainTracker& tracker) const {

    std::vector<TransitionGraph::BitmapT> pruned(tracker.size());
    const auto& original_domains = tracker.domains();

    for (VariableIdx var:_monotonicity.monotonic_variables()) {

        // For each monotonic variable, we collect the values that are locally consistent.
        // This might not be too efficient, but Gecode doesn't seem to provide a better way
        // to inspect which values are consistent.
        auto& domain = pruned[var];
        if (_translator.is_indexed(var)) {

            // Start with an all-0 bitset of appropriate size
            domain = TransitionGraph::BitmapT(original_domains[var].size(), 0);

            // And then flip to 1 those values in the domain which remain in the csp-pruned domains
            const auto& intvar = _translator.resolveInputStateVariable(csp, var);
            for (Gecode::IntVarValues i(intvar); i(); ++i) {
                int value = i.val();
                if (value < 0) throw UnimplementedFeatureException("Not yet ready for non-natural integer values");

                domain[static_cast<unsigned>(value)] = true;
            }
        }
        else {
            // The variable is not indexed by the translator because it is involved in the goal
            // only as a nested fluent. ATM we simply copy the domain from the parent
            //assert(0); // This shouldn't be happening?
            domain = original_domains[var];
        }
    }
    return DomainTracker(std::move(pruned));
}


GecodeSpace* MonotonicityCSP::
check_consistency(GecodeSpace* csp, bool stick_to_solution) const {

    if (!csp) return nullptr;

    if (!csp->propagate()) {
        delete csp;
        return nullptr;
    }

//    std::cout << "\n(Locally consistent) monotonicity CSP: " << std::endl;
//    _translator.print(std::cout, *csp);

    if (!_approximate || stick_to_solution) {  // Solve the CSP completely
        GecodeSpace* solution = solve_csp(csp);
        if (!solution) {
            delete csp;
            return nullptr;
        }
        // TODO Do something with the solution? Cache it?

        std::cout << "\nComplete solution to monotonicity CSP: " << std::endl;
        _translator.print(std::cout, *solution);

        if (stick_to_solution) {
            delete csp;
            return solution;
        }

        // ATM we just ignore it (but act differently based on whether there was a solution or not!)
        delete solution;
    }

    return csp;
}

GecodeSpace*
MonotonicityCSP::solve_csp(GecodeSpace* csp) {
    Gecode::DFS<GecodeSpace> engine(csp);
    return engine.next();
}



MonotonicityCSP*
build_monotonicity_csp(const Problem& problem, const Config& config) {
    const auto& transitions = problem.get_transition_graphs();
    if (transitions.empty() || config.getOption<bool>("ignore_transitions", false)) return nullptr;

    return new gecode::MonotonicityCSP(problem.getGoalConditions()->clone(),
                                       problem.get_tuple_index(),
                                       transitions,
                                       config.getOption<bool>("mnt.complete", false));
}

} } // namespaces
