
#pragma once

#include <fs0_types.hxx>
#include <gecode/int.hh>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <languages/fstrips/language.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>


namespace fs = fs0::language::fstrips;

namespace fs0 { class GroundAction; }


namespace fs0 { namespace gecode {

class GecodeRPGLayer;

//! The base interface class for all gecode CSP handlers
class GecodeCSPHandler {
public:
	typedef GecodeCSPHandler* ptr;
	typedef const GecodeCSPHandler* cptr;

	GecodeCSPHandler() : _base_csp(), _translator(_base_csp), _novelty(nullptr) {}
	virtual ~GecodeCSPHandler() {
		delete _novelty;
	}
	
	void init(const RPGData* bookkeeping);

	//! Create a new action CSP constraint by the given RPG layer domains
	//! Ownership of the generated pointer belongs to the caller
	SimpleCSP::ptr instantiate_csp(const GecodeRPGLayer& layer) const;
	
	const GecodeCSPVariableTranslator& getTranslator() const { return _translator; }

	static void registerTermVariables(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator);
	static void registerTermVariables(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, GecodeCSPVariableTranslator& translator);

	static void registerTermConstraints(const fs::Term::cptr term, CSPVariableType type, GecodeCSPVariableTranslator& translator);
	static void registerTermConstraints(const std::vector<fs::Term::cptr>& terms, CSPVariableType type, GecodeCSPVariableTranslator& translator);

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeCSPHandler& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const {
		return _translator.print(os, _base_csp);
	}
	
protected:
	//! The base Gecode CSP
	SimpleCSP _base_csp;

	//! A translator to map planning variables with gecode variables
	GecodeCSPVariableTranslator _translator;
	
	NoveltyConstraint* _novelty;
	
	void registerFormulaVariables(const fs::AtomicFormula::cptr condition);
	void registerFormulaVariables(const std::vector<fs::AtomicFormula::cptr>& conditions);

	void registerFormulaConstraints(const fs::AtomicFormula::cptr condition);
	void registerFormulaConstraints(const std::vector<fs::AtomicFormula::cptr>& conditions);
	
	virtual void create_novelty_constraint() = 0;
};


//! A CSP modeling and solving a logical formula on a certain RPG layer
class GecodeFormulaCSPHandler : public GecodeCSPHandler {
public:
	typedef GecodeFormulaCSPHandler* ptr;
	typedef const GecodeFormulaCSPHandler* cptr;

	GecodeFormulaCSPHandler(const std::vector<fs::AtomicFormula::cptr>& conditions);
	~GecodeFormulaCSPHandler() {}

	//! Returns true iff the goal CSP is solvable. In that case, extracts the goal supports from the first solution
	bool compute_support(SimpleCSP* csp, std::vector<Atom>& support, const State& seed) const;

	//! Simply checks if the given CSP has at least one solution
	bool check_solution_exists(SimpleCSP* csp) const;

	//! Recovers an approximate support for the goal
	void recoverApproximateSupport(SimpleCSP* csp, std::vector<Atom>& support, const State& seed) const;


protected:
	//! The formula being translated
	const std::vector<fs::AtomicFormula::cptr>& _conditions;
	
	//! 'formula_nested_fluents' contains all the nested-fluent terms of the formula
	std::vector<fs::FluentHeadedNestedTerm::cptr> formula_nested_fluents;

	//! Creates the SimpleCSP that corresponds to a certain amount of relevant variables
	void createCSPVariables();
	
	//! Preprocess the action to store the IDs of direct and indirect state variables
	void index_scopes();
	
	void create_novelty_constraint();

};

//! A CSP modeling and solving the effect of an action on a certain RPG layer
class GecodeActionCSPHandler : public GecodeCSPHandler {
public:
	typedef GecodeActionCSPHandler* ptr;
	typedef const GecodeActionCSPHandler* cptr;

	//!
	GecodeActionCSPHandler(const GroundAction& action);
	GecodeActionCSPHandler(const GroundAction& action, const std::vector<ActionEffect::cptr>& effects);
	virtual ~GecodeActionCSPHandler() {}

	const GroundAction& getAction() const { return _action; }

	void compute_support(SimpleCSP* csp, unsigned actionIdx, RPGData& rpg, const State& seed) const;
	void compute_approximate_support(SimpleCSP* csp, unsigned int action_idx, RPGData rpg, const State& seed);

protected:
	const GroundAction& _action;
	
	//! The effects of the action that we want to take into account in the CSP (by default, all)
	//! Note that we store a copy of the vector to facilitate creating subsets of effects to the subclasses.
	const std::vector<ActionEffect::cptr> _effects;
	
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

	//! Creates the SimpleCSP that corresponds to a given action.
	void createCSPVariables();

	// Variable registration methods
	void registerEffectVariables(const fs::ActionEffect::cptr effect);

	// Constraint registration methods
	void registerEffectConstraints(const fs::ActionEffect::cptr effect);
	
	//! Preprocess the action to store the IDs of direct and indirect state variables
	void index_scopes();
	
	//! Process the given solution arising from the given solution of the action CSP
	void process_solution(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping) const;
	
	//!
	void simple_atom_processing(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx) const;
	
	void hmax_based_atom_processing(SimpleCSP* solution, unsigned actionIdx, RPGData& bookkeeping, const Atom& atom, unsigned effect_idx) const;

	
	//! Extracts the full support of a given effect corresponding to the given solution
	Atom::vctrp extract_support_from_solution(SimpleCSP* solution, unsigned effect_idx) const;
	
	//!
	void create_novelty_constraint();
};

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class GecodeEffectCSPHandler : public GecodeActionCSPHandler {
public:
	typedef GecodeEffectCSPHandler* ptr;
	typedef const GecodeEffectCSPHandler* cptr;

	GecodeEffectCSPHandler(const GroundAction& action, unsigned effect_idx)
	 : GecodeActionCSPHandler(action, {action.getEffects().at(effect_idx)}), _effect_idx(effect_idx) {}
	~GecodeEffectCSPHandler() {}

protected:
	unsigned _effect_idx;
};

//! A CSP modeling and solving the progression between two RPG layers
class GecodeLayerCSPHandler : public GecodeCSPHandler { // TODO
public:
	typedef GecodeLayerCSPHandler* ptr;
	typedef const GecodeLayerCSPHandler* cptr;

protected:

};

} } // namespaces
