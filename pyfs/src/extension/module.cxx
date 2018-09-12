
#include <fs/core/fstrips/language.hxx>
#include <fs/core/base.hxx>

#include <boost/python.hpp>

#include <iostream>
#include <fstream>

namespace bp = boost::python;

namespace fs = fs0::fstrips;


// The name used in BOOST_PYTHON_MODULE must match the name of the .so library you generate and import into python.
BOOST_PYTHON_MODULE( _pyfs )
{
    bp::enum_<fs0::type_id>("type_id")
        .value("invalid_t", fs0::type_id::invalid_t)
        .value("object_t", fs0::type_id::object_t)
        .value("bool_t", fs0::type_id::bool_t)
        .value("int_t", fs0::type_id::int_t)
        .value("float_t", fs0::type_id::float_t)
    ;

    // bp::class_<fs0::object_id>("object_id", bp::init<fs0::type_id, >())
    bp::class_<fs0::object_id>("object_id", bp::no_init)
        .add_property("type", &fs0::object_id::type)
        .add_property("value", &fs0::object_id::value)
        .def(bp::self_ns::str(bp::self))
    ;

    fs0::object_id (*mo_b)(const bool&)     = &fs0::make_object;
    fs0::object_id (*mo_i)(const int32_t&)  = &fs0::make_object;
    fs0::object_id (*mo_f)(const float&)    = &fs0::make_object;
    fs0::object_id (*mo_d)(const double&)   = &fs0::make_object;

    bp::def("make_object", mo_b);
    bp::def("make_object", mo_i);
    bp::def("make_object", mo_f);
    bp::def("make_object", mo_d);

    bp::enum_<fs::Connective>("Connective")
        .value("Conjunction", fs::Connective::Conjunction)
        .value("Disjunction", fs::Connective::Disjunction)
        .value("Negation", fs::Connective::Negation)
    ;


    bp::enum_<fs::Quantifier>("Quantifier")
        .value("Universal", fs::Quantifier::Universal)
        .value("Existential", fs::Quantifier::Existential)
    ;


    bp::class_<fs::LogicalElement, boost::noncopyable>("LogicalElement", bp::no_init);
    bp::class_<fs::Term, bp::bases<fs::LogicalElement>, boost::noncopyable>("Term", bp::no_init);
    bp::class_<fs::Formula, bp::bases<fs::LogicalElement>, boost::noncopyable>("Formula", bp::no_init);


    bp::class_<fs::LogicalVariable, bp::bases<fs::Term>>("LogicalVariable", bp::init<unsigned, const std::string&, fs0::TypeIdx>())
        .add_property("id", &fs::LogicalVariable::getId)
        .add_property("name", bp::make_function(&fs::LogicalVariable::getName, bp::return_value_policy<bp::reference_existing_object>()))
        .add_property("type", &fs::LogicalVariable::getType)
        .def(bp::self_ns::str(bp::self))
    ;


    bp::class_<fs::Constant, bp::bases<fs::Term>>("Constant", bp::init<fs0::object_id, fs0::TypeIdx>())
        .add_property("value", &fs::Constant::getValue)
        .add_property("type", &fs::Constant::getType)
        .def(bp::self_ns::str(bp::self))
    ;

    bp::class_<fs::CompositeTerm, bp::bases<fs::Term>>("CompositeTerm", bp::init<unsigned, const std::vector<const fs::Term*>&>())
        .add_property("symbol", &fs::CompositeTerm::getSymbolId)
        .add_property("children", bp::make_function(&fs::CompositeTerm::getChildren, bp::return_value_policy<bp::reference_existing_object>()))
        .def(bp::self_ns::str(bp::self))
    ;


    bp::class_<fs::Tautology, bp::bases<fs::Formula>>("Tautology", bp::init<>())
        .def(bp::self_ns::str(bp::self))
    ;

    bp::class_<fs::Contradiction, bp::bases<fs::Formula>>("Contradiction", bp::init<>())
        .def(bp::self_ns::str(bp::self))
    ;

    bp::class_<fs::CompositeFormula, bp::bases<fs::Formula>>("CompositeFormula", bp::init<fs::Connective, const std::vector<const fs::Formula*>&>())
        .add_property("connective", &fs::CompositeFormula::getConnective)
        .add_property("children", bp::make_function(&fs::CompositeFormula::getChildren, bp::return_value_policy<bp::reference_existing_object>()))
        .def(bp::self_ns::str(bp::self))
    ;


    bp::class_<fs::QuantifiedFormula, bp::bases<fs::Formula>>("QuantifiedFormula", bp::init<fs::Quantifier, const std::vector<const fs::LogicalVariable*>&, const fs::Formula*>())
        .add_property("quantifier", &fs::QuantifiedFormula::getQuantifier)
        .add_property("subformula", bp::make_function(&fs::QuantifiedFormula::getSubformula, bp::return_value_policy<bp::reference_existing_object>()))
        .add_property("variables", bp::make_function(&fs::QuantifiedFormula::getVariables, bp::return_value_policy<bp::reference_existing_object>()))
        .def(bp::self_ns::str(bp::self))
    ;
}
