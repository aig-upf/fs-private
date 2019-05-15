
#include "factories.hxx"
#include "utils.hxx"

#include <fs/core/fstrips/problem.hxx>
#include <fs/core/fstrips/interpretation.hxx>



fs::AtomicFormula* create_atomic_formula(unsigned symbol_id, bp::list& subterms) {
    // We could simply convert the list into a vector, but that would result in dangling refs,
    // as both Python and the C++ planner API would assume ownership of the pointers.
    // return new fs::AtomicFormula(symbol_id, to_std_vector<const fs::Term*>(subterms));

    // Indeed, we need to clone the subterms, as the C++ API demands transfer of ownership.
    return new fs::AtomicFormula(symbol_id, clone_list<const fs::Term>(subterms));

    // We could alternatively transfer the pointer ownership (not too much tested yet), but this
    // would leave the Python list in an inconsistent state (e.g. could be that other Python variables
    // are making reference to the same object, etc.
    // return new fs::AtomicFormula(symbol_id, convert_to_vector_and_transfer_ownership<const fs::Term>(subterms));
}

fs::CompositeTerm* create_composite_term(unsigned symbol_id, bp::list& subterms) {
    return new fs::CompositeTerm(symbol_id, clone_list<const fs::Term>(subterms));
}

fs::CompositeFormula* create_composite_formula(fs::Connective connective, bp::list& subformulas) {
    return new fs::CompositeFormula(connective, clone_list<const fs::Formula>(subformulas));
}

fs::QuantifiedFormula* create_quantified_formula(fs::Quantifier quantifier, bp::list& variables, const fs::Formula* subformula) {
    return new fs::QuantifiedFormula(quantifier, clone_list<const fs::LogicalVariable>(variables), subformula->clone());
}

fs::AtomicEffect* create_atomic_effect(const fs::AtomicFormula* atom, fs::AtomicEffect::Type type, const fs::Formula* condition) {
    return new fs::AtomicEffect(atom->clone(), type, condition->clone());
}

fs::FunctionalEffect* create_functional_effect(const fs::CompositeTerm* lhs, const fs::Term* rhs, const fs::Formula* condition) {
    return new fs::FunctionalEffect(lhs->clone(), rhs->clone(), condition->clone());
}

fs::ActionSchema* create_action_schema(unsigned id, const std::string& name, bp::list& signature,
                                       bp::list& parameter_names, const fs::Formula* precondition,
                                       bp::list& effects) {
    const auto signature_ = to_std_vector<unsigned>(signature);
    const auto names_ = to_std_vector<std::string>(parameter_names);
    const auto effects_ = clone_list<const fs::ActionEffect>(effects);
    auto res = new fs::ActionSchema(id, name, signature_, names_, precondition->clone(), effects_);
    return res;
}

std::shared_ptr<fs::Problem> create_problem(const std::string& name, const std::string& domain_name,
                                            bp::list& schemas, const std::shared_ptr<fs::Interpretation> init, const fs::Formula* goal) {
    return std::make_shared<fs::Problem>(name, domain_name, clone_list<const fs::ActionSchema>(schemas), *init, goal->clone());
}