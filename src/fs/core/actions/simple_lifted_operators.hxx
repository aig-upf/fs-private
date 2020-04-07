
#pragma once

#include <fs/core/fs_types.hxx>

#include <utility>

namespace fs0 {

class PartiallyGroundedAction;
class ProblemInfo;
class State;
class Atom;

class SimpleLiftedOperator {
public:
    // An atom v = c is represented as a vector of integers <x1, x2, ..., xn> plus an unsigned value y.
    // `y` represents the value c (tipically 0 or 1, for propositional problems); whereas state variable v
    // is the result of grounding predicate (or function) symbol `x1`

    enum class term_t {var, constant};

    union var_or_object_t {
        unsigned varidx;
        object_id o;
        var_or_object_t(unsigned varidx) : varidx(varidx) {}
        var_or_object_t(object_id o) : o(o) {}
    };

    // A simple term is a function-free term, i.e., a constant or a variable. We encode that with a union to save some
    // space. We could probably use a bitfield to compact the whole thing into a word-size 64 bytes, but that would not
    // be worth the possible portability headaches.
    struct simple_term {
        term_t type;
        var_or_object_t val;
        simple_term(term_t type, var_or_object_t val) : type(type), val(val) {}
    };

    // A (lifted) atom made up of a predicate ID A, some arguments B, and a simple term t; representing the
    // atom A(B) = t or A(B) != t (depending on the value of `negated`), where B is a list of simple terms.
    struct atom_t {
        bool negated;
        uint16_t predicate_id;
        std::vector<simple_term> arguments;

        simple_term value;

        atom_t(uint16_t predicate_id, std::vector<simple_term> arguments, simple_term value, bool negated=false) :
            negated(negated), predicate_id(predicate_id), arguments(std::move(arguments)), value(value)
        {}
    };

    // A simple_term_equality represents any atom of the sort of X = Y, X != Y, X = c, c != d, etc., i.e. an (in-)equality
    // over simple terms, which are variables or constants.
    enum class simple_term_equality_t {eq, neq};
    struct simple_term_equality {
        simple_term_equality_t type;
        simple_term lhs;
        simple_term rhs;
        simple_term_equality(simple_term_equality_t type, simple_term lhs, simple_term rhs) :
            type(type), lhs(lhs), rhs(rhs)
        {}
        bool is_eq() const { return type == simple_term_equality_t::eq; }
    };

    // A condition stands for a conjunction of literals, which we represent as vector of (lifted) atoms
    struct condition_t {
        std::vector<simple_term_equality> simpleeqs;
        std::vector<atom_t> fluents;
    };

    struct effect_t {
        condition_t condition;
        atom_t atom;

        effect_t(condition_t condition, atom_t atom) :
                condition(std::move(condition)), atom(std::move(atom))
        {}
    };


    //! A null operator that does nothing. We need this to act as a null value in certain contexts.
    SimpleLiftedOperator() :
            precondition(), effects()
    {}

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
        const std::vector<object_id>& binding,
        const ProblemInfo& info,
        bool check_precondition,
        std::vector<Atom>& atoms);

} // namespaces
