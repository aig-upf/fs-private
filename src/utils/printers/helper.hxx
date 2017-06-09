
#pragma once

#include <vector>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <fs_types.hxx>


namespace fs0 {
//! Helper to generate std::strings from printable objects
//! Adapted from: http://stackoverflow.com/a/12262626
//! Example usage:
//! throw std::runtime_error(printer() << foo << 13 << ", bar" << myData);   // implicitly cast to std::string
//! throw std::runtime_error(printer() << foo << 13 << ", bar" << myData >> printer::to_str);    // explicitly cast to std::string
class printer {
public:
	printer() = default;
	~printer() = default;
	printer(const printer&) = delete;
	printer& operator=(printer&) = delete;

	template <typename Type>
	printer & operator<<(const Type & value) {
		stream_ << value;
		return *this;
	}

	std::string str() const         { return stream_.str(); }
	operator std::string() const   { return stream_.str(); }

	enum ConvertToString { to_str };
	std::string operator>>(ConvertToString) { return stream_.str(); }

private:
    std::stringstream stream_;
};
} // namespaces

namespace fs0 { namespace print {



class Helper {
public:
	static const std::vector<std::string> name_variables(const std::vector<VariableIdx>& variables);
	static const std::vector<std::string> name_objects(const std::vector<object_id>& objects);
};

	
template<class T> 
std::string to_string(const T& t, bool *ok = NULL) { 
	std::ostringstream stream;
	stream << t;
	if(ok != NULL) *ok = (stream.fail() == false);
	return stream.str();
}

template<class T> 
std::string normalize(const T& t) {
	std::string s = fs0::print::to_string(t);
	std::replace(s.begin(), s.end(), '-', '_');
	s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
	return s;
}


} } // namespaces

