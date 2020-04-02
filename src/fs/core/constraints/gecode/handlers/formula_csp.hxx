
#pragma once

#include <fs/core/constraints/gecode/handlers/base_csp.hxx>

namespace fs0 { class AtomIndex; }
namespace fs0 { namespace language { namespace fstrips { class Formula; }}}
namespace fs = fs0::language::fstrips;


namespace fs0 { namespace gecode {
	
//! A CSP modeling and solving a logical formula on a certain RPG layer
class FormulaCSP : public BaseCSP {
public:

	FormulaCSP(const fs::Formula* formula, const AtomIndex& tuple_index, bool approximate);
	~FormulaCSP();
	FormulaCSP(const FormulaCSP&) = delete;
	FormulaCSP(FormulaCSP&&) = delete;
	FormulaCSP& operator=(const FormulaCSP&) = delete;
	FormulaCSP& operator=(FormulaCSP&&) = delete;
	
	//! Returns true iff the goal CSP is solvable. In that case, extracts the goal supports from the first solution
	bool compute_support(FSGecodeSpace* csp, std::vector<AtomIdx>& support) const;
	
	void init_value_selector(const RPGIndex* graph);
	
	//! Return true iff the CSP has at least one solution
	bool is_satisfiable(FSGecodeSpace* csp) const;

	static FSGecodeSpace* compute_single_solution(FSGecodeSpace* csp);

protected:
	//! The formula being managed
	const fs::Formula* _formula;

	//! A list with all tuples that are relevant to the action effect. The first element of the pair
	//! is the index of the symbol, then come the indexes of the subterms (Indexes are CSP variable indexes).
	std::vector<std::pair<unsigned, std::vector<unsigned>>> _tuple_indexes;
	
	void index_scopes();
	
	void create_novelty_constraint() {}
	
	void index();
	
	void index_existential_variable_uses();
};

} } // namespaces
