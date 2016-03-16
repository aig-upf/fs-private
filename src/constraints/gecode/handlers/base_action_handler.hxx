
#pragma once

#include <constraints/gecode/handlers/base_handler.hxx>

namespace fs0 { class BaseAction; class ActionID; }

namespace fs0 { namespace language { namespace fstrips { class ActionEffect; }}}


namespace fs0 { namespace gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class BaseActionCSPHandler : public BaseCSPHandler {
public:
	typedef BaseActionCSPHandler* ptr;

	//! Constructor / Destructor
	BaseActionCSPHandler(const BaseAction& action, const std::vector<const fs::ActionEffect*>& effects, bool approximate, bool use_novelty_constraint);
	virtual ~BaseActionCSPHandler() {}
	
	//!
	virtual void process(const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;

	//! Initialize the value selector of the underlying CSPs
	virtual void init_value_selector(const RPGData* bookkeeping) { init(bookkeeping);} // TODO - No need to have two different names for the same method
	
	const BaseAction& get_action() const { return _action; }
	
	const std::vector<const fs::ActionEffect*>& get_effects() const { return _effects; }
	
	virtual bool find_atom_support(const Atom& atom, const State& seed, const GecodeRPGLayer& layer, RPGData& rpg) const;
	
	virtual void post(SimpleCSP& csp, const Atom& atom) const;
	
	
protected:
	//! The index of the action managed by this manager
	const BaseAction& _action;
	
	//! The effects of the action that we want to take into account in the CSP (by default, all)
	//! Note that we store a copy of the vector to facilitate creating subsets of effects to the subclasses.
	const std::vector<const fs::ActionEffect*> _effects;
	
	//! 'effect_support_variables[i]' contains the scope of the i-th effect of the action plus the scope of the action, without repetitions
	//! and in that particular order.
	std::vector<VariableIdxVector> effect_support_variables;
	
	//! 'effect_nested_fluents[i]' contains all the nested-fluent terms of the RHS of the i-th effect plus those of the action precondition,
	//! in that particular order
	std::vector<std::vector<const fs::FluentHeadedNestedTerm*>> effect_nested_fluents;
	
	//! 'effect_rhs_variables[i]' contains the index of the CSP variable that models the value of the RHS of the i-th effect.
	std::vector<unsigned> effect_rhs_variables;
	
	//! Whether the action has any effect with a LHS that contains nested fluents.
	bool _has_nested_lhs;
	
	//! Whether the action has any nested relevant term.
	bool _has_nested_relevant_terms;
	
	//! Whether to use the min-hmax-sum policy to prioritize the different supports of the same atom
	bool _hmaxsum_priority;
	
	//! When _has_nested_lhs is false, we store here the VariableIdx referred to by the LHS of each effect, which can be deduced statically
	std::vector<VariableIdx> effect_lhs_variables;
	
	//! See parent class description
	void index();
	
	//! Preprocess the action to store the IDs of direct and indirect state variables
	virtual void index_scopes();
	
	void compute_support(SimpleCSP* csp, RPGData& rpg, const State& seed) const;
	
	std::set<VariableIdx> _action_support;
	
	// Constraint registration methods
	void registerEffectConstraints(const fs::ActionEffect* effect);
	
	//! Process the given solution of the action CSP
	void process_solution(SimpleCSP* solution, RPGData& bookkeeping) const;
	
	//!
	void simple_atom_processing(SimpleCSP* solution, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const;
	
	void hmax_based_atom_processing(SimpleCSP* solution, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const;

	//! Extracts the full support of a given effect corresponding to the given solution
	Atom::vctrp extract_support_from_solution(SimpleCSP* solution, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const;
	
	//!
	void create_novelty_constraint();
	
	//! Return the ActionID that corresponds to the current action / action schema, for some given solution
	virtual const ActionID* get_action_id(SimpleCSP* solution) const = 0;
	
	//! Return the action binding that corresponds to the given solution - by default, return an empty binding
	virtual Binding build_binding_from_solution(SimpleCSP* solution) const;
	
	//! A simple helper to log the processing message
	virtual void log() const = 0;
};

} } // namespaces
