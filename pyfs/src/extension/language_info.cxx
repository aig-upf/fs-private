
#include "utils.hxx"
#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/base.hxx>

#include <boost/python.hpp>

#include <iostream>


namespace bp = boost::python;
namespace fs = fs0::fstrips;


//! Adds a symbol with the given information to the given LanguageInfo object, and returns the symbol ID
//! `signature` must be a list of registered type_ids
fs::symbol_id add_symbol_to_language_info(const std::string& name, fs::symbol_t symbol_type, bp::list& signature, fs::LanguageInfo& lang) {
    // std::cout << "Adding symbol " << name << std::endl;
    const auto signature_vector = to_std_vector<fs0::TypeIdx>(signature);
    return lang.add_symbol(name, symbol_type, signature_vector);
}


void define_language_info() {

    enum class symbol_t {Predicate, Function};

    bp::enum_<fs::symbol_t>("symbol_t")
            .value("Predicate", fs::symbol_t::Predicate)
            .value("Function", fs::symbol_t::Function)
    ;

    bp::def("add_symbol_to_language_info", &add_symbol_to_language_info);




//    bool    (X::*fx1)(int)              = &X::f;


    fs0::TypeIdx (fs::LanguageInfo::*add_fstype1)(const std::string&, fs0::type_id)                       = &fs::LanguageInfo::add_fstype;
    fs0::TypeIdx (fs::LanguageInfo::*add_fstype2)(const std::string&, fs0::type_id, const fs0::type_range&)  = &fs::LanguageInfo::add_fstype;
//    fs0::TypeIdx (*add_fstype2)(const int32_t&)  = &fs0::add_fstype;


    const std::string& (fs::LanguageInfo::*get_typename1)(const fs0::TypeIdx&) const  = &fs::LanguageInfo::get_typename;


    bp::class_<fs::LanguageInfo>("LanguageInfo", bp::init<>())
            .def("get_symbol_id", &fs::LanguageInfo::get_symbol_id)
            .def("get_symbol_name", &fs::LanguageInfo::get_symbol_name, bp::return_value_policy<bp::copy_const_reference>())

            .def("get_fstype_id", &fs::LanguageInfo::get_fstype_id)
            .def("get_typename", get_typename1, bp::return_value_policy<bp::copy_const_reference>())

            .def("get_object_name", &fs::LanguageInfo::get_object_name)


            // ...

            .def("add_symbol", &fs::LanguageInfo::add_symbol)
            .def("add_object", &fs::LanguageInfo::add_object)
            .def("add_fstype", add_fstype1)
            .def("add_fstype", add_fstype2)

//            .add_property("name", bp::make_function(&fs::ActionSchema::getName, bp::return_value_policy<bp::reference_existing_object>()))
//            .def(bp::self_ns::str(bp::self))
    ;


}
