
#pragma once

#include <vector>
#include <clingo/clingocontrol.hh>

namespace fs0 { namespace asp {

class Clingo {
protected:
	
// 	std::vector<char const *> _args;
	
// 	DefaultGringoModule _module;
	
// 	Gringo::Scripts _scripts;
	
// 	ClingoLib _lib;
	
public:
	typedef std::vector<Gringo::Value> Solution;
	Clingo();
	
	std::pair<Gringo::SolveResult, Solution> solve(const std::vector<std::string>& base, const std::vector<std::string>& state);

// 	void test();

};

} } // namespaces
