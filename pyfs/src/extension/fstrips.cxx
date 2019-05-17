
#include "factories.hxx"

#include <fs/core/fstrips/language.hxx>
#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/fstrips/problem.hxx>
#include <fs/core/fstrips/grounding.hxx>
#include <fs/core/fstrips/interpretation.hxx>
#include <fs/core/base.hxx>


#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <signal.h>

namespace bp = boost::python;
namespace fs = fs0::fstrips;

template <typename T>
std::string _print(const T& o, const fs::LanguageInfo& info) {
    std::ostringstream oss;
    o.print(oss, info);
    return oss.str();
}

std::string print_logical_element(const fs::LogicalElement& o, const fs::LanguageInfo& info) {
    return _print(o, info);
}

std::string print_effect(const fs::ActionEffect& o, const fs::LanguageInfo& info) {
    return _print(o, info);
}

//std::string print_action(const fs::ActionSchema& o, const fs::LanguageInfo& info) {
//    return _print(o, info);
//}


void define_fstrips() {
    /// First-Order Logic Language ///

    bp::enum_<fs0::type_id>("type_id")
            .value("invalid_t", fs0::type_id::invalid_t)
            .value("object_t", fs0::type_id::object_t)
            .value("bool_t", fs0::type_id::bool_t)
            .value("int_t", fs0::type_id::int_t)
            .value("float_t", fs0::type_id::float_t)
            ;

    bp::class_<fs0::object_id>("object_id", bp::no_init)
            .add_property("type", &fs0::object_id::type)
            .add_property("value", &fs0::object_id::value)
            .def(bp::self_ns::str(bp::self))
            ;

    fs0::object_id (*mo_b)(const bool&)     = &fs0::make_object;
    fs0::object_id (*mo_i)(const int32_t&)  = &fs0::make_object;
    fs0::object_id (*mo_f)(const float&)    = &fs0::make_object;
    //fs0::object_id (*mo_d)(const double&)   = &fs0::make_object;

    bp::def("make_object", mo_b);
    bp::def("make_object", mo_i);
    bp::def("make_object", mo_f);
    //bp::def("make_object", mo_d);

    bp::enum_<fs::Connective>("Connective")
            .value("And", fs::Connective::Conjunction)
            .value("Or", fs::Connective::Disjunction)
            .value("Not", fs::Connective::Negation)
            ;


    bp::enum_<fs::Quantifier>("Quantifier")
            .value("Exists", fs::Quantifier::Universal)
            .value("Forall", fs::Quantifier::Existential)
    ;


    bp::class_<fs::LogicalElement, boost::noncopyable>("LogicalElement", bp::no_init)
        .def("print", &print_logical_element)
    ;

    bp::class_<fs::Term, bp::bases<fs::LogicalElement>, boost::noncopyable>("Term", bp::no_init);
    bp::class_<fs::Formula, bp::bases<fs::LogicalElement>, boost::noncopyable>("Formula", bp::no_init);


    bp::class_<fs::LogicalVariable, bp::bases<fs::Term>>("LogicalVariable", bp::init<unsigned, const std::string&, fs0::TypeIdx>())
            .add_property("id", &fs::LogicalVariable::getId)
            .add_property("name", bp::make_function(&fs::LogicalVariable::getName, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("type", &fs::LogicalVariable::getType)
//            .def(bp::self_ns::str(bp::self))
            ;


    bp::class_<fs::Constant, bp::bases<fs::Term>>("Constant", bp::init<fs0::object_id, fs0::TypeIdx>())
            .add_property("value", &fs::Constant::getValue)
            .add_property("type", &fs::Constant::getType)
//            .def(bp::self_ns::str(bp::self))
            ;

    bp::class_<fs::CompositeTerm, bp::bases<fs::Term>>("CompositeTerm", bp::init<unsigned, const std::vector<const fs::Term*>&>())
            .add_property("symbol", &fs::CompositeTerm::getSymbolId)
            .add_property("children", bp::make_function(&fs::CompositeTerm::getChildren, bp::return_value_policy<bp::reference_existing_object>()))
//            .def(bp::self_ns::str(bp::self))
            ;


    bp::class_<fs::Tautology, bp::bases<fs::Formula>>("Tautology", bp::init<>())
//            .def(bp::self_ns::str(bp::self))
            ;

    bp::class_<fs::Contradiction, bp::bases<fs::Formula>>("Contradiction", bp::init<>())
//            .def(bp::self_ns::str(bp::self))
            ;

    // Declare a few helpers to deal with constructors involving vectors
    bp::def("create_atomic_formula", &create_atomic_formula, bp::return_value_policy<bp::manage_new_object>());
    bp::def("create_composite_formula", &create_composite_formula, bp::return_value_policy<bp::manage_new_object>());
    bp::def("create_composite_term", &create_composite_term, bp::return_value_policy<bp::manage_new_object>());
    bp::def("create_quantified_formula", &create_quantified_formula, bp::return_value_policy<bp::manage_new_object>());

    bp::def("create_action_schema", &create_action_schema, bp::return_value_policy<bp::manage_new_object>());
    bp::def("create_atomic_effect", &create_atomic_effect, bp::return_value_policy<bp::manage_new_object>());
    bp::def("create_functional_effect", &create_functional_effect, bp::return_value_policy<bp::manage_new_object>());

    bp::def("create_problem", &create_problem);


    bp::class_<fs::AtomicFormula, bp::bases<fs::Formula>>("AtomicFormula", bp::no_init)
            .add_property("symbol", &fs::CompositeTerm::getSymbolId)
            .add_property("children", bp::make_function(&fs::AtomicFormula::getChildren, bp::return_value_policy<bp::reference_existing_object>()))
                    //.def(bp::self_ns::str(bp::self))
//            .def("__str__", &print_wrapper)
            ;


    bp::class_<fs::CompositeFormula, bp::bases<fs::Formula>>("CompositeFormula", bp::init<fs::Connective, const std::vector<const fs::Formula*>&>())
            .add_property("connective", &fs::CompositeFormula::getConnective)
            .add_property("children", bp::make_function(&fs::CompositeFormula::getChildren, bp::return_value_policy<bp::reference_existing_object>()))
//            .def(bp::self_ns::str(bp::self))
            ;


    bp::class_<fs::QuantifiedFormula, bp::bases<fs::Formula>>("QuantifiedFormula", bp::init<fs::Quantifier, const std::vector<const fs::LogicalVariable*>&, const fs::Formula*>())
            .add_property("quantifier", &fs::QuantifiedFormula::getQuantifier)
            .add_property("subformula", bp::make_function(&fs::QuantifiedFormula::getSubformula, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("variables", bp::make_function(&fs::QuantifiedFormula::getVariables, bp::return_value_policy<bp::reference_existing_object>()))
//            .def(bp::self_ns::str(bp::self))
            ;

    /// FSTRIPS Actions ///

    bp::enum_<fs::AtomicEffect::Type>("AtomicEffectType")
            .value("Add", fs::AtomicEffect::Type::ADD)
            .value("Del", fs::AtomicEffect::Type::DEL)
            ;

    bp::class_<fs::ActionEffect, boost::noncopyable>("ActionEffect", bp::no_init)
            .def("print", &print_effect)
//            .def(bp::self_ns::str(bp::self))
            ;

    bp::class_<fs::FunctionalEffect, bp::bases<fs::ActionEffect>>("FunctionalEffect", bp::init<const fs::CompositeTerm*, const fs::Term*, const fs::Formula*>())
            .add_property("lhs", bp::make_function(&fs::FunctionalEffect::lhs, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("rhs", bp::make_function(&fs::FunctionalEffect::rhs, bp::return_value_policy<bp::reference_existing_object>()))
        //.def(bp::self_ns::str(bp::self))
            ;

    bp::class_<fs::AtomicEffect, bp::bases<fs::ActionEffect>>("AtomicEffect", bp::init<const fs::AtomicFormula*, const fs::AtomicEffect::Type, const fs::Formula*>())
            .add_property("atom", bp::make_function(&fs::AtomicEffect::getAtom, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("type", &fs::AtomicEffect::getType)
        //.def(bp::self_ns::str(bp::self))
            ;

    bp::class_<fs::ActionSchema>("ActionSchema", bp::init<unsigned, const std::string&, const fs0::Signature&, const std::vector<std::string>&, const fs::Formula*, const std::vector<const fs::ActionEffect*>&>())
            .add_property("name", bp::make_function(&fs::ActionSchema::getName, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("signature", bp::make_function(&fs::ActionSchema::getSignature, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("parameters", bp::make_function(&fs::ActionSchema::getParameterNames, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("precondition", bp::make_function(&fs::ActionSchema::getPrecondition, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("effects", bp::make_function(&fs::ActionSchema::getEffects, bp::return_value_policy<bp::reference_existing_object>()))
//            .def("print", &print_action) // TODO This still ends up invoking the singleton ProblemInfo, so will result in segfaults
//            .def(bp::self_ns::str(bp::self))
    ;

    bp::class_<fs::Interpretation, std::shared_ptr<fs::Interpretation>>("Interpretation", bp::init<>())
    ;

    bp::class_<fs::Problem, std::shared_ptr<fs::Problem>>("Problem", bp::init<const std::string&, const std::string&, const std::vector<const fs::ActionSchema*>&, const fs::Interpretation&, const fs::Formula*>())
            .add_property("name", bp::make_function(&fs::Problem::name, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("domain_name", bp::make_function(&fs::Problem::domain_name, bp::return_value_policy<bp::reference_existing_object>()))
//            .add_property("schemas", bp::make_function(&fs::Problem::schemas, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("init", bp::make_function(&fs::Problem::init, bp::return_value_policy<bp::reference_existing_object>()))
            .add_property("goal", bp::make_function(&fs::Problem::goal, bp::return_value_policy<bp::reference_existing_object>()))
    ;

    bp::class_<fs::Grounding, std::shared_ptr<fs::Grounding>>("Grounding", bp::init<std::shared_ptr<fs::LanguageInfo>>())
        .def("add_state_variable", &fs::Grounding::add_state_variable)
    ;
}
