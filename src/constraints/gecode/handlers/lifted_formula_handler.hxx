
#pragma once

#include <constraints/gecode/handlers/base_handler.hxx>

namespace fs0 { class TupleIndex; }
namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;


namespace fs0 { namespace gecode {
	
//! A CSP modeling and solving a logical formula on a certain RPG layer
class LiftedFormulaHandler : public BaseCSPHandler {
public:

	LiftedFormulaHandler(const fs::Formula* formula, const TupleIndex& tuple_index, bool approximate);
	~LiftedFormulaHandler();

	//! Returns true iff the goal CSP is solvable. In that case, extracts the goal supports from the first solution
	bool compute_support(SimpleCSP* csp, std::vector<TupleIdx>& support, const State& seed) const;
	
	//! Simply checks if the given CSP has at least one solution
	bool check_solution_exists(SimpleCSP* csp) const;

	SimpleCSP::ptr instantiate(const RPGIndex& rpg) const;
	
	void init_value_selector(const RPGIndex* graph);

protected:
	//! The formula being managed
	const fs::Formula* _formula;

	//! A list with all tuples that are relevant to the action effect. The first element of the pair
	//! is the index of the symbol, then come the indexes of the subterms (Indexes are CSP variable indexes).
	std::vector<std::pair<unsigned, std::vector<unsigned>>> _tuple_indexes;
	
	void index_scopes();
	
	void create_novelty_constraint() {}
	
	void index();
};

} } // namespaces
