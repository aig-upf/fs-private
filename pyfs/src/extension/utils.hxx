

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

//! Convert any (iterable) Python object into an equivalent std::vector
template<typename T> inline
std::vector<T> to_std_vector(const bp::object& iterable) {
    return std::vector<T>(bp::stl_input_iterator<T>(iterable), bp::stl_input_iterator<T>());
}

/*
template<typename T> inline
std::vector<T*> convert_to_vector_and_transfer_ownership(const bp::list& list) {
    std::vector<T*> result;
    for (int i = 0; i < len(list); ++i) {
        const auto& elem = list[i];
        std::auto_ptr<T> ptr = boost::python::extract<std::auto_ptr<T>>(elem);
//        std::unique_ptr<T> ptr = boost::python::extract<std::unique_ptr<T>>(elem);
        result.push_back(ptr.release());
    }
    return result;
}
*/

template<typename T> inline
std::vector<T*> clone_list(const bp::list& list) {
    std::vector<T*> result;
    for (int i = 0; i < len(list); ++i) {
        const T* elem = boost::python::extract<T*>(list[i]);
//        auto ptr = boost::python::extract<T*>(elem);
//        auto ptr = std::auto_ptr<T>(elem);
//        auto ptr = std::unique_ptr<T>(elem);
        result.push_back(elem->clone());
    }
    return result;
}