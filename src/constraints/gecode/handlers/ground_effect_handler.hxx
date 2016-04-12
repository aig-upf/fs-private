
#pragma once

#include <constraints/gecode/handlers/base_action_handler.hxx>

namespace fs0 {
class GroundAction;
}

namespace fs0 { namespace gecode {

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class GroundEffectCSPHandler : public BaseActionCSPHandler {
public:
	//! Factory method
	static std::vector<std::shared_ptr<GroundEffectCSPHandler>> create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty);

	GroundEffectCSPHandler(const GroundAction& action, const TupleIndex& tuple_index, unsigned effect_idx, bool approximate);
	 
	~GroundEffectCSPHandler() {}
	
	bool init(bool use_novelty_constraint) override;
	
	const fs::ActionEffect* get_effect() const { 
		assert(_effects.size() == 1);
		return _effects[0];
	}
	
	//! Preinstantiate the CSP
	SimpleCSP* preinstantiate(const RPGIndex& rpg) const;
	
	bool find_atom_support(TupleIdx tuple, const Atom& atom, const State& seed, SimpleCSP& layer_csp, RPGIndex& rpg) const;
	
	void post(SimpleCSP& csp, const Atom& atom) const;
	
protected:
	const ActionID* get_action_id(const SimpleCSP* solution) const;

	SimpleCSP* instantiate_effect_csp(const RPGIndex& rpg) const;

		
	//! Index the CSP variables corresponding the the effect LHS.
	std::vector<unsigned> index_lhs_subterms();
	
	//! '_lhs_subterm_variables[i]' is the index of the CSP variable corresponding to the i-th subterm of the effect LHS.
	std::vector<unsigned> _lhs_subterm_variables;

		
	//! A list with all tuples that are relevant to the action effect. The first element of the pair
	//! is the index of the symbol, then come the indexes of the subterms (Indexes are CSP variable indexes).
	std::vector<std::pair<unsigned, std::vector<unsigned>>> _tuple_indexes;
	
	void log() const;
	
	bool solve(TupleIdx tuple, gecode::SimpleCSP* csp, RPGIndex& graph) const;
// 	void solve_approximately(const Atom& atom, gecode::SimpleCSP* csp, RPGData& rpg, const State& seed) const;
};


} } // namespaces
