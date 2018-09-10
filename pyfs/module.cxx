
#include <fs/core/fstrips/language.hxx>

#include <iostream>
#include <fstream>

#include <boost/python.hpp>

//#include <python_runner.hxx>

namespace fs = fs0::fstrips;


namespace bp = boost::python;

// The name used in BOOST_PYTHON_MODULE must match the name of the .so library you generate and import into python.
BOOST_PYTHON_MODULE( _pyfs )
{
    bp::class_<fs::LogicalElement, boost::noncopyable>("LogicalElement", bp::no_init)

    ;
}
