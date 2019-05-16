
#include "factories.hxx"

#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/fstrips/language.hxx>
#include <fs/core/fstrips/interpretation.hxx>
#include <fs/core/fstrips/problem.hxx>
#include <fs/core/base.hxx>
#include <fs/core/lambda/states.hxx>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>



namespace bp = boost::python;
namespace fs = fs0::fstrips;
namespace lmb = lambda;


//! Search-related classes
void define_search() {

//    bp::class_<lmb::State, boost::noncopyable>("State", bp::no_init)
//    ;

//    bp::class_<lmb::BinaryState, bp::bases<lmb::State>>("BinaryState", bp::init<const lmb::BinaryState::bitset_t&>())
//    ;

}
