
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
        _partial_extensions(),
        _monotonic_variables()
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

std::vector<TransitionGraph::BitmapT>
TransitionGraph::retrieve_domains(const std::vector<Atom>& changeset) const {
    std::vector<TransitionGraph::BitmapT> result;
    result.reserve(changeset.size());

    for (const auto& atom:changeset) {
        const VariableIdx& var = atom.getVariable();
        if (!is_monotonic(var)) {
            result.emplace_back();
            continue;
        }

        const auto& all_domains = _reachable_bitsets[var];
        const auto& it = all_domains.find(atom.getValue());
        if (it == all_domains.end()) {
            throw std::runtime_error("Monotonicity domain mapping is wrong - "
                                     "it should have a set of allowed transitions for any possible value in the domain of any monotonic variable");
        }

        result.push_back(it->second); // copy the domain into the result
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
TransitionGraph::compute_reachable_sets(const TransitionGraphT& transitions) {
    std::unordered_map<object_id, std::unordered_set<object_id>> reach;

    // Start by collecting all initial transitions
    for (const auto& transition:transitions) {
        // If no set was constructed for transition.first, this will empty-construct a new one
        reach[transition.first].insert(transition.second);
    }

    // Build the transitive closure until a fixpoint is reached
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

    // Any value is reachable from itself, by definition
    for (auto& r:reach) {
        r.second.insert(r.first);
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
        result[origin] = Gecode::IntSet(values.data(), values.size());

        // DEBUG
//        std::cout << "Reachable from " << origin << ":\t";
//        for (int v:values) std::cout << v << ", ";
//        std::cout << std::endl;
    }

    return result;
}

std::unordered_map<object_id, TransitionGraph::BitmapT>
TransitionGraph::generate_bitset(
        int max_value,
        const std::unordered_map<object_id, std::unordered_set<object_id>>& rechable) {

    // Now transform into the desired IntSet-based representation:
    std::unordered_map<object_id, BitmapT> result;

    // Iterate through pairs <x, R>, where R is the set of object IDs that
    // are reachable from object ID 'x'
    for (const auto& elem:rechable) {
        const auto& origin = elem.first;

        BitmapT bm(max_value+1, 0);
        for (const auto& oid:elem.second) {
            int value = fs0::value<int>(oid);
            if (value < 0) {
                throw UnimplementedFeatureException("Negative int variables not supported for monotonic constraints");
            }
            assert(value <= max_value);
            bm[value] = true;
        }

        result.insert(std::make_pair(origin, bm));

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

void TransitionGraph::preprocess_extensions(const AllTransitionGraphsT& transitions) {
    const auto& info = ProblemInfo::getInstance();

    for (unsigned var = 0; var < transitions.size(); ++var) {
        const auto& transition = transitions[var];

        int max = 0;

        // If the variable has some transition defined, we consider it monotonic
        if (!transition.empty()) {
            _monotonic_variables.insert(var);
            const auto& int_values = fs0::values<int>(info.getVariableObjects(var), ObjectTable::EMPTY_TABLE);
            max = *std::max_element(std::begin(int_values), std::end(int_values));
        }

        _reachable.push_back(compute_reachable_sets(transition));
        _allowed_domains.push_back(preprocess_extension(_reachable[var]));
        _reachable_bitsets.push_back(generate_bitset(max, _reachable[var]));
        _partial_extensions.push_back(precompute_partial_extensions(var, _reachable[var]));
    }
}

std::vector<int> as_vector(const TransitionGraph::BitmapT& bs) {
    std::vector<int> res;
    for (size_t index = bs.find_first(); index != TransitionGraph::BitmapT::npos; index = bs.find_next(index)) {
        res.push_back(static_cast<int>(index));
    }
    return res;
}


std::vector<Gecode::IntSet> DomainTracker::to_intsets() const {
    assert(!is_null());

    std::vector<Gecode::IntSet> res;
    res.reserve(_domains.size());

    for (const auto& dom:_domains) {
        std::vector<int> values = as_vector(dom);
        res.emplace_back(values.data(), values.size());
    }

    return res;
}

std::ostream& print::bitset::print(std::ostream& os) const {
    const auto& info = ProblemInfo::getInstance();
    os << "\t" << info.getVariableName(_var) << ": {";

    for (size_t index = _bitset.find_first(); index != TransitionGraph::BitmapT::npos;) {
        const auto oid = fs0::make_object(info.sv_type(_var), static_cast<int>(index));
        os << info.object_name(oid);

        index = _bitset.find_next(index);
        if (index != TransitionGraph::BitmapT::npos) os << ", ";
    }
    os << "}" << std::endl;
    return os;
}

std::ostream& print::domain_tracker::print(std::ostream& os) const {
    const auto& domains = _domains.domains();
    os << "DomainTracker:" << std::endl;
    for (unsigned var = 0; var < domains.size(); ++var) {
        os << print::bitset(var, domains[var]);
    }
    return os << std::endl;
}

} // namespaces