
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
	static std::vector<std::shared_ptr<BaseActionCSPHandler>> create(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_index, bool approximate, bool novelty);

	GroundEffectCSPHandler(const GroundAction& action, const TupleIndex& tuple_index, unsigned effect_idx, bool approximate);
	 
	~GroundEffectCSPHandler() {}
	
	virtual void init(bool use_novelty_constraint);
	
	const fs::ActionEffect* get_effect() const { 
		assert(_effects.size() == 1);
		return _effects[0];
	}
	
	//! Preinstantiate the CSP
	SimpleCSP* preinstantiate(const GecodeRPGLayer& layer) const;
	
	bool find_atom_support(const Atom& atom, const State& seed, SimpleCSP& csp, RPGData& rpg) const;
	
	void post(SimpleCSP& csp, const Atom& atom) const;
	
protected:
	const ActionID* get_action_id(const SimpleCSP* solution) const;

	//! Index the CSP variables corresponding the the effect LHS.
	std::vector<unsigned> index_lhs_subterms();
	
	//! '_lhs_subterm_variables[i]' is the index of the CSP variable corresponding to the i-th subterm of the effect LHS.
	std::vector<unsigned> _lhs_subterm_variables;

	void log() const;
	
	bool solve_completely(gecode::SimpleCSP* csp, RPGData& rpg) const;
	void solve_approximately(const Atom& atom, gecode::SimpleCSP* csp, RPGData& rpg, const State& seed) const;
};


} } // namespaces
