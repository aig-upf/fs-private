
#pragma once

#include <boost/lexical_cast.hpp>

// Specialize boost::lexical_cast to handle things such as boost::lexical_cast<bool>("true") gracefully.  See http://stackoverflow.com/a/17708833
namespace boost {
    template<> 
    inline bool lexical_cast<bool, std::string>(const std::string& arg) {
        std::istringstream ss(arg);
        bool b;
        ss >> std::boolalpha >> b;
        return b;
    }

    template<>
    inline std::string lexical_cast<std::string, bool>(const bool& b) {
        std::ostringstream ss;
        ss << std::boolalpha << b;
        return ss.str();
    }
}
