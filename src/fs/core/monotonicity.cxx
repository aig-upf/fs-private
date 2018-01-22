
#include <fs/core/monotonicity.hxx>
#include <fs/core/state.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/problem_info.hxx>

namespace fs0 {

TransitionGraph::TransitionGraph(const AtomIndex &tuple_index,
                                 const AllTransitionGraphsT &transitions) :
        _tuple_index(tuple_index),
        _transitions(transitions),
        _reachable(),
        _allowed_domains(),
        _partial_extensions()
{
    preprocess_extensions(transitions);
    precompute_full_extensions();
}

void TransitionGraph::precompute_full_extensions() {
    const ProblemInfo& info = ProblemInfo::getInstance();
    for (VariableIdx var = 0; var < info.getNumVariables(); ++var) {
        PartialExtensionT var_extension;

//        if (info.isPredicativeVariable(var)) {

//        } else {
            const auto& domain = info.getVariableObjects(var);
            for (const auto& val:domain) {
                const ValueTuple& values = _tuple_index.to_tuple(var, val);
                var_extension.emplace_back(
                        fs0::values<int>(values, ObjectTable::EMPTY_TABLE));
            }
//        }

        _full_extensions.push_back(var_extension);
    }
}


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


std::unordered_map<object_id, std::unordered_set<object_id>>
TransitionGraph::compute_reachable_sets(const TransitionGraphT &transitions) {
    std::unordered_map<object_id, std::unordered_set<object_id>> reach;

    // Start by collecting all initial transitions
    for (const auto& transition:transitions) {
        // If no set was constructed for transition.first, this will empty-construct a new one:
        reach[transition.first].insert(transition.second);
    }


    // And then build the transitive closure until a fixpoint is reached
    for(bool fixpoint = false; !fixpoint; ) {
        fixpoint = true;

        for (auto& element:reach) {
            const object_id& v1 = element.first;
            _unused(v1);
            std::unordered_set<object_id>& reachable_from_v1 = element.second;
            std::unordered_set<object_id> tmp(reachable_from_v1); // copy the set
            std::size_t size = reachable_from_v1.size();

            for(const object_id& v2:reachable_from_v1) {
                assert(v1 != v2);
                const auto& reachable_from_v2 = reach[v2];
                tmp.insert(reachable_from_v2.cbegin(), reachable_from_v2.cend());
            }

            if (tmp.size() != size) {
                fixpoint = false;
                reachable_from_v1 = tmp;
            }
        }
    }
    return reach;
}


std::unordered_map<object_id, Gecode::IntSet>
TransitionGraph::preprocess_extension(const std::unordered_map<object_id, std::unordered_set<object_id>>& rechable) {

    // Now transform into the desired IntSet-based representation:
    std::unordered_map<object_id, Gecode::IntSet> result;

    // Iterate through pairs <x, R>, where R is the set of object IDs that
    // are reachable from object ID 'x'
    for (const auto& elem:rechable) {
        const auto& origin = elem.first;

        std::vector<object_id> oids(elem.second.cbegin(), elem.second.cend());

        std::vector<int> values = fs0::values<int>(oids, ObjectTable::EMPTY_TABLE);
        // The origin value is also "reachable" by definition, i.e. we want to allow it
        // in the extensional constraints
        values.push_back(fs0::value<int>(origin));
        result[origin] = Gecode::IntSet(values.data(), values.size());

        // DEBUG
//        std::cout << "Reachable from " << origin << ":\t";
//        for (int v:values) std::cout << v << ", ";
//        std::cout << std::endl;
    }

    return result;
}


std::unordered_map<object_id, TransitionGraph::PartialExtensionT>
TransitionGraph::precompute_partial_extensions(VariableIdx var, const std::unordered_map<object_id, std::unordered_set<object_id>>& rechable) {

    std::unordered_map<object_id, PartialExtensionT> extensions_for_var;

    // Iterate through pairs <x, R>, where R is the set of object IDs that
    // are reachable from object ID 'x'
    for (const auto& elem:rechable) {
        const auto& origin = elem.first;
        auto& extensions_for_var_from_origin = extensions_for_var[origin];


        for (const object_id& reachable_val:elem.second) {
            const ValueTuple& values = _tuple_index.to_tuple(var, reachable_val);
            extensions_for_var_from_origin.emplace_back(Gecode::IntArgs(fs0::values<int>(values, ObjectTable::EMPTY_TABLE)));
        }

        // The origin value is also "reachable" by definition, i.e. we want to allow it
        // in the extensional constraints
        const ValueTuple& values = _tuple_index.to_tuple(var, origin);
        extensions_for_var_from_origin.emplace_back(Gecode::IntArgs(fs0::values<int>(values, ObjectTable::EMPTY_TABLE)));

//        // DEBUG
//        std::cout << "Reachable from " << origin << ":\t";
//        for (int v:values) std::cout << v << ", ";
//        std::cout << std::endl;
    }

    return extensions_for_var;
}

bool TransitionGraph::is_active() const {
    return !_transitions.empty();
}

bool TransitionGraph::has_defined_transitions(VariableIdx var) const {
    return !_transitions.at(var).empty();
}


const TransitionGraph::PartialExtensionT&
TransitionGraph::compute_partial_extension(VariableIdx var,
                                           const object_id &val) const {

    // If the variable has no transition defined upon it, we must return as
    // possible extension all the tuples given (statically) by the variable
    // domain
    if (!has_defined_transitions(var)) {
        return _full_extensions[var];
    }

    const auto& var_extensions = _partial_extensions.at(var);
    const auto& it = var_extensions.find(val);

    if (it == var_extensions.end()) {
        throw std::runtime_error("Monotonicity domain mapping is wrong - "
                                 "Should have a set of partial extensions for "
                                 "any possible value in the domain of any monotonic variable");
    }

    return it->second;
}

void TransitionGraph::preprocess_extensions(const AllTransitionGraphsT &transitions) {

    for (const auto& transition:transitions) {
        auto var = static_cast<VariableIdx>(_reachable.size());
        _reachable.push_back(compute_reachable_sets(transition));
        _allowed_domains.push_back(preprocess_extension(_reachable[var]));
        _partial_extensions.push_back(precompute_partial_extensions(var, _reachable[var]));
    }
}

} // namespaces