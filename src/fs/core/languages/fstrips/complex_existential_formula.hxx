
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/utils/atom_index.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>


namespace fs0 { namespace language { namespace fstrips {

//! An Existential formula with a CSP-based evaluation strategy.
// TODO - The choice of evaluation strategy should be independent
// TODO - of the AST node itself. And should definitely not belong
// TODO - to the FSTRIPS language module itself
class ComplexExistentialFormula : public Formula {
public:
    LOKI_DEFINE_CONST_VISITABLE();

    explicit ComplexExistentialFormula(const Formula* formula, const AtomIndex& tuple_index) :
            _interpreter(formula->clone(), tuple_index)
    {}

    ~ComplexExistentialFormula() override = default;

    ComplexExistentialFormula* clone() const override { throw UnimplementedFeatureException(""); }

    bool interpret(const PartialAssignment& assignment, Binding& binding) const override { throw UnimplementedFeatureException(""); }
    bool interpret(const State& state, Binding& binding) const override {
        return _interpreter.satisfied(state);
    }
    using Formula::interpret;

    std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const override {
        return os << "ComplexExistentialFormula";
    }

protected:
    CSPFormulaInterpreter _interpreter;
};

} } } // namespaces