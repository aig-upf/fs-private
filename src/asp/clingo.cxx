
#include <iostream>
#include <vector>
#include <stdexcept>

#include <asp/clingo.hxx>
#include <aptk2/tools/logging.hxx>
#include <utils/printers/helper.hxx>

namespace fs0 { namespace asp {

Clingo::Clingo() 
// 	: _args({"clingo", "-e", "brave", nullptr}), _module(), 
// 	_scripts(_module),
// 	_lib(_scripts, _args.size() - 2, _args.data())
{}

	
/*
void Clingo::test() { 
	_lib.add("base", {}, "a :- not b. b :- not a.");
	_lib.ground({{"base", {}}}, nullptr);
	_lib.solve([](Gringo::Model const &m) {
		
		for (auto &atom : m.atoms(Gringo::Model::SHOWN)) {
			std::cout << atom << " "; 
		}
		std::cout << std::endl;
		return true; 
	}, {});
}
*/

std::pair<Gringo::SolveResult, Clingo::Solution> Clingo::solve(const std::vector<std::string>& base, const std::vector<std::string>& state) {

	std::vector<char const *> args{"clingo", nullptr};
	DefaultGringoModule module;
	Gringo::Scripts scripts(module);
	ClingoLib lib(scripts, args.size() - 2, args.data());
	
	for (const auto& rule:base) lib.add("base", {}, rule);
	for (const auto& rule:state) lib.add("base", {}, rule);
	
	lib.ground({{"base", {}}}, nullptr);
	
	Solution solution;
	
	Gringo::SolveResult result = lib.solve([&solution](Gringo::Model const &m) {
		solution = m.atoms(Gringo::Model::SHOWN);
		return true;
	}, {});
	
	return std::make_pair(result, solution);
}


} } // namespaces