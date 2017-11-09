
#pragma once

#include <fs/core/constraints/gecode/handlers/lifted_action_csp.hxx>
#include <gecode/int.hh>

namespace fs0 { class AtomIndex; }

namespace fs0 { namespace language { namespace fstrips { class StateVariable; } } }
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class RPGIndex;

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class LiftedEffectUnreachedCSP : public LiftedActionCSP {
public:
	//! Factory methods
	static std::vector<std::unique_ptr<LiftedEffectUnreachedCSP>> create(const std::vector<const PartiallyGroundedAction*>& schemata, const AtomIndex& tuple_index, bool approximate, bool novelty);
	
	//! The only constructor
	LiftedEffectUnreachedCSP(const PartiallyGroundedAction& action, const fs::ActionEffect* effect, const AtomIndex& tuple_index, bool approximate);
	~LiftedEffectUnreachedCSP() = default;
	LiftedEffectUnreachedCSP(const LiftedEffectUnreachedCSP&) = delete;
	LiftedEffectUnreachedCSP(LiftedEffectUnreachedCSP&&) = delete;
	LiftedEffectUnreachedCSP& operator=(const LiftedEffectUnreachedCSP&) = delete;
	LiftedEffectUnreachedCSP& operator=(LiftedEffectUnreachedCSP&&) = delete;
	
	bool init(bool use_novelty_constraint) override;

	const fs::ActionEffect* get_effect() const;
	
	const fs::Formula* get_precondition() const override;

protected:
	
	//! A list with all tuples that are relevant to the action effect. The first element of the pair
	//! is the index of the symbol, then come the indexes of the subterms (Indexes are CSP variable indexes).
	std::vector<std::pair<unsigned, std::vector<unsigned>>> _tuple_indexes;
	
	void create_novelty_constraint() override;
	
	void post_novelty_constraint(GecodeCSP& csp, const RPGIndex& rpg) const override;

	
	void log() const override;
	
	
	
	bool solve_for_tuple(AtomIdx tuple, gecode::GecodeCSP* csp, RPGIndex& graph) const;
	
	void post_atom(GecodeCSP& csp, const Atom& atom) const;


	//! Index the CSP variables corresponding the the effect LHS.
	std::vector<unsigned> index_lhs_subterms();
	
	//! '_lhs_subterm_variables[i]' is the index of the CSP variable corresponding to the i-th subterm of the effect LHS.
	std::vector<unsigned> _lhs_subterm_variables;

public:
	//! Preinstantiate the CSP
	GecodeCSP* preinstantiate(const RPGIndex& rpg) const;
	
	//! Find whether this effect can support the atom 'tuple' = 'atom' in the RPG layer given by layer_csp
	bool find_atom_support(AtomIdx tuple, const Atom& atom, const State& seed, GecodeCSP& layer_csp, RPGIndex& rpg) const;
};


} } // namespaces
