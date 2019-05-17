
#include "factories.hxx"

#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/fstrips/language.hxx>
#include <fs/core/fstrips/interpretation.hxx>
#include <fs/core/fstrips/problem.hxx>
#include <fs/core/base.hxx>
//#include <fs/core/lambda/states.hxx>
#include <fs/core/lambda/search/search_model.hxx>
#include <fs/core/lambda/search/factory.hxx>
//#include <fs/core/lambda/search/breadth_first_search.hxx>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>



namespace bp = boost::python;
namespace fs = fs0::fstrips;
namespace ls = lambda::search;


//! Search-related classes
void define_search() {

//    bp::class_<lmb::State, boost::noncopyable>("State", bp::no_init)
//    ;

//    bp::class_<lmb::BinaryState, bp::bases<lmb::State>>("BinaryState", bp::init<const lmb::BinaryState::bitset_t&>())
//    ;

    bp::class_<ls::SearchModel, std::shared_ptr<ls::SearchModel>, boost::noncopyable>("SearchModel", bp::no_init)
    ;

    bp::class_<ls::SearchAlgorithm, std::shared_ptr<ls::SearchAlgorithm>, boost::noncopyable>("SearchAlgorithm", bp::no_init)
    ;

    bp::def("create_model", &create_model);

    bp::def("create_breadth_first_search_engine", &create_breadth_first_search_engine);

}
