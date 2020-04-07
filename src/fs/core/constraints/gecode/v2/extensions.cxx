
#include "extensions.hxx"

#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <fs/core/utils/binding_iterator.hxx>


namespace fs0::gecode::v2 {

IndividualSymbolExtensionGenerator::IndividualSymbolExtensionGenerator(unsigned symbol_id, const ProblemInfo& info, const AtomIndex& tuple_index) :
        symbol_id(symbol_id),
        arity(0) // we'll initialize this below
{
    const auto& symbol_data = info.getSymbolData(symbol_id);
    bool is_predicate = info.isPredicate(symbol_id);
    arity = symbol_data.getUniformArity();

    for (utils::binding_iterator it(symbol_data.getSignature(), info); !it.ended(); ++it) {
        auto binding = *it;
        const std::vector<object_id>& point = binding.get_full_binding();
        std::vector<int> values = fs0::values<int>(point, ObjectTable::EMPTY_TABLE);


        if (info.is_fluent(symbol_id, point)) { // We have a fluent atom
            VariableIdx var = info.resolveStateVariable(symbol_id, point);
            for (const object_id& value:info.getVariableObjects(var)) {
                if (value != object_id::FALSE) {
                    if (!is_predicate) {
                        assert(o_type(value) == type_id::int_t || o_type(value) == type_id::object_t);
                        values.push_back(fs0::value<int>(value));
                    }
                    fluent_tuples.emplace_back(var, value, Gecode::IntArgs(values));
                }
            }

        } else { // We have a static atom
            Function f = symbol_data.getFunction();
            object_id value = f(point);

            if (is_predicate) {
                if (value == object_id::TRUE) {
                    static_tuples.emplace_back(values);
                }
            } else {
                assert(o_type(value) == type_id::int_t || o_type(value) == type_id::object_t);
                values.push_back(fs0::value<int>(value));
                static_tuples.emplace_back(values);
            }
        }
    }
}


Gecode::TupleSet IndividualSymbolExtensionGenerator::instantiate(const State& state) const {
    Gecode::TupleSet ts((int) arity);

    for (const auto& tuple:static_tuples) {
        ts.add(tuple);
    }

    for (const auto& elem:fluent_tuples) {
        if (state.getValue(std::get<0>(elem)) == std::get<1>(elem)) {
            ts.add(std::get<2>(elem));
        }
    }

    ts.finalize();
    return ts;
}

IndividualSymbolExtensionGenerator::IndividualSymbolExtensionGenerator() :
    symbol_id(std::numeric_limits<unsigned>::max()), arity(std::numeric_limits<unsigned>::max())
{
}


SymbolExtensionGenerator::SymbolExtensionGenerator(
        const ProblemInfo& info,
        const AtomIndex& tuple_index,
        std::vector<unsigned> managed) :
    managed(managed)
{
    for (unsigned s=0; s < managed.size(); ++s) {
        if ((bool) managed[s]) {
            individuals.emplace_back(s, info, tuple_index);
        } else {
            // NOLINTNEXTLINE  (the nullary constructor is protected, hence cannot use emplace_back)
            individuals.push_back(IndividualSymbolExtensionGenerator());
        }
    }
}

std::vector<Gecode::TupleSet> SymbolExtensionGenerator::instantiate(const State& state) const {
    std::vector<Gecode::TupleSet> result(managed.size());

    auto sz = managed.size();
    for (unsigned s=0; s < sz; ++s) {
        if ((bool) managed[s]) {
            result[s] = individuals[s].instantiate(state);
        }
    }

    return result;
}

} // namespaces
