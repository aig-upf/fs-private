
#include <fs/core/fstrips/language.hxx>

#include <boost/python.hpp>

#include <iostream>
#include <fstream>

namespace bp = boost::python;

namespace fs = fs0::fstrips;


// The name used in BOOST_PYTHON_MODULE must match the name of the .so library you generate and import into python.
BOOST_PYTHON_MODULE( _pyfs )
{
    bp::class_<fs::LogicalElement, boost::noncopyable>("LogicalElement", bp::no_init)

    ;
}
