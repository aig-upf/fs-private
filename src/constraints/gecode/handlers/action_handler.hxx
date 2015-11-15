
#pragma once

#include <constraints/gecode/handlers/csp_handler.hxx>

namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
template <typename ActionT>
class GecodeElementCSPHandler : public GecodeCSPHandler {
public:
	typedef GecodeElementCSPHandler* ptr;

	//!
	GecodeElementCSPHandler(const ActionT& action, bool use_novelty_constraint);
	GecodeElementCSPHandler(const ActionT& action, const std::vector<fs::ActionEffect::cptr>& effects, bool use_novelty_constraint);
	virtual ~GecodeElementCSPHandler() {}

	const ActionT& getAction() const { return _action; }

	void compute_support(SimpleCSP* csp, unsigned actionIdx, RPGData& rpg, const State& seed) const;
	void compute_approximate_support(SimpleCSP* csp, unsigned int action_idx, RPGData rpg, const State& seed);

protected:
	const ActionT& _action;
	
	//! The effects of the action that we want to take into account in the CSP (by default, all)
	//! Note that we store a copy of the vector to facilitate creating subsets of effects to the subclasses.
	const std::vector<fs::ActionEffect::cptr> _effects;
	
	//! 'effect_support_variables[i]' contains the scope of the i-th effect of the action plus the scope of the action, without repetitions
	//! and in that particular order.
	std::vector<VariableIdxVector> effect_support_variables;
	
	//! 'effect_nested_fluents[i]' contains all the nested-fluent terms of the RHS of the i-th effect plus those of the action precondition,
	//! in that particular order
	std::vector<std::vector<fs::FluentHeadedNestedTerm::cptr>> effect_nested_fluents;
	
	//! 'effect_rhs_variables[i]' contains the index of the CSP variable that models the value of the RHS of the i-th effect.
	std::vector<unsigned> effect_rhs_variables;
	
	//! Whether the action has any effect with a LHS that contains nested fluents.
	bool _has_nested_lhs;
	
	//! Whether to use the min-hmax-sum policy to prioritize the different supports of the same atom
	bool _hmaxsum_priority;
	
	//! When _has_nested_lhs is true, we store here the VariableIdx referred to by the LHS of each effect, which can be deduced statically
	std::vector<VariableIdx> effect_lhs_variables;

	// Constraint registration methods
	void registerEffectConstraints(const fs::ActionEffect::cptr effect);
	
	//! Preprocess the action to store the IDs of direct and indirect state variables
	void index_scopes();
	
	//! Process the given solution of the action CSP
	void process_solution(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping) const;
	
	//!
	void simple_atom_processing(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx) const;
	
	void hmax_based_atom_processing(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx) const;

	
	//! Extracts the full support of a given effect corresponding to the given solution
	Atom::vctrp extract_support_from_solution(SimpleCSP* solution, unsigned effect_idx) const;
	
	//!
	void create_novelty_constraint();
	
	void index();
};

typedef GecodeElementCSPHandler<GroundAction> GecodeActionCSPHandler;

} } // namespaces
