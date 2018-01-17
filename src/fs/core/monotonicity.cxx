
#include <fs/core/monotonicity.hxx>
#include <fs/core/state.hxx>
#include <fs/core/atom.hxx>

#include <gecode/int.hh>

namespace fs0 {


const std::vector<const Gecode::IntSet*>
TransitionGraph::compute_domains(const std::vector<Atom>& changeset) const {
    std::vector<const Gecode::IntSet*> result(_allowed_domains.size(), nullptr);

    for (const auto& atom:changeset) {
        const VariableIdx& var = atom.getVariable();
        assert(var < result.size());
        const auto& all_domains_for_variable = _allowed_domains[var];

        // If the map is empty, we understand that there are no monotonicity
        // constraints placed over this particular variable
        if (all_domains_for_variable.empty()) continue;

        // Otherwise, we just register the appropriate IntSet
        const auto& it = all_domains_for_variable.find(atom.getValue());
        if (it == all_domains_for_variable.end()) {
            throw std::runtime_error("Monotonicity domain mapping is wrong - "
                                             "it should have a set of allowed transitions for any possible value in the domain of any monotonic variable");
        }

        result[var] = &(it->second);
    }

    return result;
}


bool TransitionGraph::transition_is_valid(VariableIdx variable, const object_id& val0, const object_id& val1) const {
    const auto& var_transitions = _transitions.at(variable);
    // If no transitions are declared on the state variable, we assume that
    // all transitions are possible
    // TODO This is not too elegant, might be better to signal in some other way
    // which variables are not affected by monotonicity constraints
    // But for the moment being, it is efficient and works
    if (var_transitions.empty()) return true;
    return var_transitions.find(std::make_pair(val0, val1)) != var_transitions.end();
}

std::vector<std::unordered_map<object_id, Gecode::IntSet>>
TransitionGraph::build(const AllTransitionGraphsT& transitions) {
    std::vector<std::unordered_map<object_id, Gecode::IntSet>> result;
    for (const auto& transition:transitions) {
        result.push_back(build_single(transition));
    }
    return result;
}

std::unordered_map<object_id, Gecode::IntSet>
TransitionGraph::build_single(const TransitionGraphT& transitions) {

    // We will iteratively accumulate in 'reachable' the transitive closure
    // of the graph given as by the edge set in `transitions`
    std::unordered_map<object_id, std::unordered_set<object_id>> reachable;

    // Start by collecting all initial transitions
    for (const auto& transition:transitions) {
        // If no set was constructed for transition.first, this will empty-construct a new one:
        reachable[transition.first].insert(transition.second);
    }


    // And then build the transitive closure until a fixpoint is reached
    for(bool fixpoint = false; !fixpoint; ) {
        fixpoint = true;

        for (auto& element:reachable) {
            const object_id& v1 = element.first;
            _unused(v1);
            std::unordered_set<object_id>& reachable_from_v1 = element.second;
            std::unordered_set<object_id> tmp(reachable_from_v1); // copy the set
            std::size_t size = reachable_from_v1.size();

            for(const object_id& v2:reachable_from_v1) {
                assert(v1 != v2);
                const auto& reachable_from_v2 = reachable[v2];
                tmp.insert(reachable_from_v2.cbegin(), reachable_from_v2.cend());
            }

            if (tmp.size() != size) {
                fixpoint = false;
                reachable_from_v1 = tmp;
            }
        }
    }

    // Now transform into the desired IntSet-based representation:
    std::unordered_map<object_id, Gecode::IntSet> result;
    for (const auto& elem:reachable) {
        std::vector<object_id> oids(elem.second.cbegin(), elem.second.cend());

        std::vector<int> values = fs0::values<int>(oids, ObjectTable::EMPTY_TABLE);
        // The origin value is also "reachable" by definition, i.e. we want to allow it
        // in the extensional constraints
        values.push_back(fs0::value<int>(elem.first));
        result[elem.first] = Gecode::IntSet(values.data(), values.size());

        // DEBUG
        std::cout << "Reachable from " << elem.first << ":\t";
        for (int v:values) std::cout << v << ", ";
        std::cout << std::endl;
    }

    return result;
}

bool TransitionGraph::is_active() const {
    return !_transitions.empty();
}

} // namespaces