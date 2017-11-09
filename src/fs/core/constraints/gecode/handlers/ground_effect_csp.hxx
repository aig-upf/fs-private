
#pragma once

#include <fs/core/constraints/gecode/handlers/base_action_csp.hxx>
#include <fs/core/actions/actions.hxx> // Necessary so that the return of get_action can be identified as covariant with that of the overriden method

namespace fs0 {
class GroundAction;
}

namespace fs0 { namespace gecode {

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class GroundEffectCSP : public BaseActionCSP {
public:
	//! Factory method
	static std::vector<std::unique_ptr<GroundEffectCSP>> create(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_index, bool approximate, bool novelty);

	GroundEffectCSP(const GroundAction& action, const AtomIndex& tuple_index, const fs::ActionEffect* effect, bool approximate, bool use_effect_conditions);
	~GroundEffectCSP() = default;
	GroundEffectCSP(const GroundEffectCSP&) = delete;
	GroundEffectCSP(GroundEffectCSP&&) = delete;
	GroundEffectCSP& operator=(const GroundEffectCSP&) = delete;
	GroundEffectCSP& operator=(GroundEffectCSP&&) = delete;
	
	bool init(bool use_novelty_constraint) override;
	
	const fs::ActionEffect* get_effect() const { 
		assert(_effects.size() == 1);
		return _effects[0];
	}
	
	//! Preinstantiate the CSP
	GecodeCSP* preinstantiate(const RPGIndex& rpg) const;
	
	//! Find whether this effect can support the atom 'tuple' = 'atom' in the RPG layer given by layer_csp
	bool find_atom_support(AtomIdx tuple, const Atom& atom, const State& seed, GecodeCSP& layer_csp, RPGIndex& rpg) const;
	
	void post(GecodeCSP& csp, const Atom& atom) const;
	
	const GroundAction& get_action() const override { return _action; }
	
	const std::vector<const fs::ActionEffect*>& get_effects() const override;

	const fs::Formula* get_precondition() const override;
	
protected:

	const GroundAction& _action;
	
	//! This is the only effect managed by this CSP, which we store in a vector to comply with the parents' interfaces, which require
	//! to return a vector of effects. By construction, we have that _effects.size() == 1
	const std::vector<const fs::ActionEffect*> _effects;
	
	const ActionID* get_action_id(const GecodeCSP* solution) const override;

	//! Index the CSP variables corresponding the the effect LHS.
	std::vector<unsigned> index_lhs_subterms();
	
	//! '_lhs_subterm_variables[i]' is the index of the CSP variable corresponding to the i-th subterm of the effect LHS.
	std::vector<unsigned> _lhs_subterm_variables;

		
	//! A list with all tuples that are relevant to the action effect. The first element of the pair
	//! is the index of the symbol, then come the indexes of the subterms (Indexes are CSP variable indexes).
	std::vector<std::pair<unsigned, std::vector<unsigned>>> _tuple_indexes;
	
	void log() const override;
	
	bool solve(AtomIdx tuple, gecode::GecodeCSP* csp, RPGIndex& graph) const;
// 	void solve_approximately(const Atom& atom, gecode::GecodeCSP* csp, RPGData& rpg, const State& seed) const;
};


} } // namespaces
