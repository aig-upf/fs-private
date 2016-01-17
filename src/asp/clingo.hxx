
#pragma once

#include <clingo/clingocontrol.hh>

namespace fs0 { namespace asp {

class Clingo {
protected:
	
	std::vector<char const *> _args;
	
	DefaultGringoModule _module;
	
	Gringo::Scripts _scripts;
	
	ClingoLib _lib;
	
public:
	Clingo();
	
	std::pair<Gringo::SolveResult, std::vector<Gringo::Value>> solve(const std::vector<std::string>& base, const std::vector<std::string>& state);

	void test();

};

} } // namespaces
