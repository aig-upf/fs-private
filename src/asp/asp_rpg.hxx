
#pragma once

#include <memory>
#include <asp/clingo.hxx>

namespace fs0 { class State; class Problem; }

namespace fs0 { namespace asp {

class Clingo; class Model;

template <typename RPGBaseHeuristic>
class ASPRPG {
protected:
	std::shared_ptr<Clingo> _clingo;
	
	//! The actual ASP model corresponding to the problem
	std::shared_ptr<Model> _model;
	
	//! Whether we are using ASP optimization or not
	bool _optimize;
	
public:
	ASPRPG(const Problem& problem, RPGBaseHeuristic&& heuristic, bool optimize);

	//! The actual evaluation of the heuristic value for any given non-relaxed state s.
	long evaluate(const State& seed);

};

} } // namespaces
