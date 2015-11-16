
#pragma once

#include <constraints/gecode/handlers/base_handler.hxx>

namespace fs0 { namespace gecode {

//! A CSP modeling and solving a logical formula on a certain RPG layer
class FormulaCSPHandler : public BaseCSPHandler {
public:
	typedef FormulaCSPHandler* ptr;
	typedef const FormulaCSPHandler* cptr;

	FormulaCSPHandler(const fs::Formula::cptr formula, bool approximate, bool use_novelty_constraint);
	~FormulaCSPHandler() {
		delete _formula;
	}

	//! Returns true iff the goal CSP is solvable. In that case, extracts the goal supports from the first solution
	bool compute_support(SimpleCSP* csp, std::vector<Atom>& support, const State& seed) const;
	
	//! Simply checks if the given CSP has at least one solution
	bool check_solution_exists(SimpleCSP* csp) const;

	//! Recovers an approximate support for the goal
	void recoverApproximateSupport(SimpleCSP* csp, std::vector<Atom>& support, const State& seed) const;


protected:
	//! The formula being translated
	const fs::Formula::cptr _formula;
	
	void index_scopes() {}
	
	void create_novelty_constraint();
	
	void index();
};

} } // namespaces
