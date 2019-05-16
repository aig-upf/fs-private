
#include <boost/python.hpp>

void define_language_info();
void define_fstrips();
void define_search();

// IMPORTANT: The name in BOOST_PYTHON_MODULE(.) must match that of the .so library file.
BOOST_PYTHON_MODULE( _pyfs )
{
    define_language_info();
    define_fstrips();
    define_search();
}



