
#pragma once

#include <memory>
#include <asp/clingo.hxx>

namespace fs0 { class State; class Problem; }

namespace fs0 { namespace asp {

class Clingo; class Model;

class ModelHandler {
protected:
	std::shared_ptr<Clingo> _clingo;
	
	//! The actual ASP model corresponding to the problem
	std::shared_ptr<Model> _model;
	
	//! The LP rules corresponding to the planning domain.
	const std::vector<std::string> _domain_rules;
	
public:
	ModelHandler(const Problem& problem, bool optimize);

	//! Solve and return the solution for the LP corresponding to the RPG computed on the given seed state.
	std::pair<Gringo::SolveResult, Clingo::Solution> process(const State& seed);
	
	std::vector<unsigned> get_action_set(const Clingo::Solution& model) const;
};

} } // namespaces
