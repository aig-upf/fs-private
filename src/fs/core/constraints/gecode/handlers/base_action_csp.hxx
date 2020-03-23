
#pragma once

#include <fs/core/constraints/gecode/handlers/base_csp.hxx>

namespace fs0 { class ActionBase; class ActionID; }

namespace fs0::language::fstrips { class ActionEffect; class Formula; }


namespace fs0::gecode {

class NoveltyConstraint;

//! A CSP modeling and solving the effect of an action on a certain RPG layer
class BaseActionCSP : public BaseCSP {
public:
	//! Constructor / Destructor
	BaseActionCSP(const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions);
	~BaseActionCSP() override;
	BaseActionCSP(const BaseActionCSP&) = delete;
	BaseActionCSP(BaseActionCSP&&) = delete;
	BaseActionCSP& operator=(const BaseActionCSP&) = delete;
	BaseActionCSP& operator=(BaseActionCSP&&) = delete;	
	
	//! Returns false iff the induced CSP is inconsistent, i.e. the action is not applicable
	virtual bool init(bool use_novelty_constraint);

	//!
	virtual void process(RPGIndex& graph) const;

	//! Initialize the value selector of the underlying CSPs
	virtual void init_value_selector(const RPGData* bookkeeping) { init(bookkeeping);} // TODO - No need to have two different names for the same method
	
	//! Return the action that gives origin to this CSP manager
	virtual const ActionBase& get_action() const = 0;
	
	//! Return the set of effects (possibly one) managed by this object
	virtual const std::vector<const fs::ActionEffect*>& get_effects() const = 0;
	
	//! Return the precondition managed by this object (which might be the one of the original action, or
	//! contain some extra / modified conditions e.g. in case of nested fluent processing)
	virtual const fs::Formula* get_precondition() const = 0;

protected:

	NoveltyConstraint* _novelty;
	
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
	
	bool _use_effect_conditions;
	
	//! When _has_nested_lhs is false, we store here the VariableIdx referred to by the LHS of each effect, which can be deduced statically
	std::vector<VariableIdx> effect_lhs_variables;
	
	std::set<VariableIdx> _action_support;
	
	
	//! See parent class description
	void index() override;
	
	//! Preprocess the action to store the IDs of direct and indirect state variables
	virtual void index_scopes();
	
	void compute_support(GecodeSpace* csp, RPGIndex& graph) const;
	
	// Constraint registration methods
	void registerEffectConstraints(const fs::ActionEffect* effect);
	
	//! Process the given solution of the action CSP
	void process_solution(GecodeSpace* solution, RPGIndex& graph) const;
	
	//!
	void simple_atom_processing(GecodeSpace* solution, RPGIndex& graph, AtomIdx tuple, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const;
	
// 	void hmax_based_atom_processing(GecodeSpace* solution, RPGIndex& graph, const Atom& atom, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const;

	//! Extracts the full support of a given effect corresponding to the given solution
	std::vector<AtomIdx> extract_support_from_solution(GecodeSpace* solution, unsigned effect_idx, const PartialAssignment& assignment, const Binding& binding) const;
	
	
	//! Return the ActionID that corresponds to the current action / action schema, for some given solution
	virtual const ActionID* get_action_id(const GecodeSpace* solution) const = 0;
	
	//! Return the action binding that corresponds to the given solution - by default, return an empty binding
	virtual Binding build_binding_from_solution(const GecodeSpace* solution) const;
	
	//! A simple helper to log the processing message
	virtual void log() const = 0;
	
	//!
	void create_novelty_constraint() override;
	
	//!
	void post_novelty_constraint(GecodeSpace& csp, const RPGIndex& rpg) const override;
	
	//!
	void extract_nested_term_support(const GecodeSpace* solution, const std::vector<const fs::FluentHeadedNestedTerm*>& nested_terms, const PartialAssignment& assignment, const Binding& binding, std::vector<AtomIdx>& support) const;
};

} // namespaces
