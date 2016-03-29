
#pragma once

#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <gecode/int.hh>

namespace fs0 { namespace gecode {

class IndexedTupleset;

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class EffectSchemaCSPHandler : public ActionSchemaCSPHandler {
public:
	typedef EffectSchemaCSPHandler* ptr;
	
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSPHandler>> create(const std::vector<const ActionSchema*>& schemata, const std::vector<IndexedTupleset>& symbol_tuplesets, bool approximate, bool novelty);
	//! HACK
	static std::vector<std::shared_ptr<EffectSchemaCSPHandler>> create_derived(const std::vector<const ActionSchema*>& schemata, const std::vector<IndexedTupleset>& symbol_tuplesets, bool approximate, bool novelty);

	EffectSchemaCSPHandler(const ActionSchema& action, const fs::ActionEffect* effect, bool approximate);
	 
	~EffectSchemaCSPHandler() {}
	
	void init(bool use_novelty_constraint, const IndexedTupleset& tupleset);

		
	const fs::ActionEffect* get_effect() const;
	
	unsigned get_lhs_symbol() const { return _lhs_symbol; }
	
// 	SimpleCSP* post_novelty(const SimpleCSP& csp, const Gecode::TupleSet& extension);
	
// 	bool solve(const State& seed, SimpleCSP& csp, RPGData& bookkeeping, Tupleset& extension);


	void seek_novel_tuples(const GecodeRPGLayer& layer, std::unordered_set<unsigned>& reached, RPGData& rpg, const State& seed) const;


	int seek_single_solution(SimpleCSP& csp, RPGData& bookkeeping, const State& seed) const;
	
	void register_indexing_constraint(const Gecode::TupleSet& index_extension);


protected:
	void log() const;
	
	//! In an effect f(t) := w, '_lhs_symbol' is the index of symbol 'f'
	unsigned _lhs_symbol;
	
	//! The indexes (in the CSP) of the CSP variables that correspond to the tuple 't' in an effect LHS of the form f(t) := w.
	std::vector<unsigned> _lhs_argument_variables;
	
	//! In an effect f(t) := w, '_lhs_symbol' is the index of the CSP variable corresponding to the term 'w'.
	unsigned _rhs_variable;
	
	
	std::vector<unsigned> index_lhs_variables(const fs::ActionEffect* effect);
	
	static unsigned index_lhs_symbol(const fs::ActionEffect* effect);
	
	unsigned index_rhs_term(const fs::ActionEffect* effect);

	unsigned _tuple_index_var;
	
	unsigned _novelty_bool_var;
};


} } // namespaces
