

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

//! Convert any (iterable) Python object into an equivalent std::vector
template<typename T> inline
std::vector<T> to_std_vector(const bp::object& iterable) {
    return std::vector<T>(bp::stl_input_iterator<T>(iterable), bp::stl_input_iterator<T>());
}

