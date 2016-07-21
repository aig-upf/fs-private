
#pragma once

#include <vector>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <fs_types.hxx>


namespace fs0 { namespace print {

class Helper {
public:
	static const std::vector<std::string> name_variables(const std::vector<VariableIdx>& variables);
	static const std::vector<std::string> name_objects(const std::vector<ObjectIdx>& objects, const Signature& signature);
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
