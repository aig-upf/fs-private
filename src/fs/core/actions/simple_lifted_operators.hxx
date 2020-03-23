
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/binding.hxx>
#include <fs/core/applicability/action_managers.hxx>
#include <fs/core/languages/fstrips/axioms.hxx>
#include <utility>

namespace fs0 {

class PartiallyGroundedAction;
class ProblemInfo;

class SimpleLiftedOperator {
public:
    // An atom v = c is represented as a vector of integers <x1, x2, ..., xn> plus an unsigned value y.
    // `y` represents the value c (tipically 0 or 1, for propositional problems); whereas state variable v
    // is the result of grounding predicate (or function) symbol `x1`

    enum class argtype_t {var, constant};

    union var_or_object_t {
        unsigned varidx;
        object_id o;
        var_or_object_t(unsigned varidx) : varidx(varidx) {}
        var_or_object_t(object_id o) : o(o) {}
    };

    struct argument_t {
        argtype_t type;
        var_or_object_t val;
        argument_t(argtype_t type, var_or_object_t val) : type(type), val(val) {}
    };

    // A (lifted) atom made up of a predicate ID A, some arguments B, and a constant value C; representing the
    // atom A(B) = C; where the integers b_i in B = <b1, b2, ... bn> can be either variables (b_i < 0, in which
    // case abs(b_i) represents the variable ID, or constants (b_i >= 0
    struct atom_t {
        uint16_t predicate_id;
        std::vector<argument_t> arguments;
        object_id value;

        atom_t(uint16_t predicate_id, std::vector<argument_t> arguments, object_id value) :
                predicate_id(predicate_id), arguments(std::move(arguments)), value(value)
        {}
    };

    // A condition stands for a conjunction of literals, which we represent as vector of (lifted) atoms
    using condition_t = std::vector<atom_t>;

    struct effect_t {
        std::vector<atom_t> condition;
        atom_t atom;

        effect_t(std::vector<atom_t> condition, atom_t atom) :
                condition(std::move(condition)), atom(std::move(atom))
        {}
    };


    SimpleLiftedOperator(condition_t precondition, std::vector<effect_t> effects) :
            precondition(std::move(precondition)), effects(std::move(effects))
    {}

    condition_t precondition;
    std::vector<effect_t> effects;
};

SimpleLiftedOperator compile_schema_to_simple_lifted_operator(const PartiallyGroundedAction& action);

void evaluate_simple_lifted_operator(
        const State& state,
        const SimpleLiftedOperator& op,
        const Binding& binding,
        const ProblemInfo& info,
        bool check_precondition,
        std::vector<Atom>& atoms);

} // namespaces
