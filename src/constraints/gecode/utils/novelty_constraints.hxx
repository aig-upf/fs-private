
#pragma once

#include <fs0_types.hxx>
#include <boost/functional/hash.hpp>

namespace fs0 { namespace language { namespace fstrips { class Formula; class ActionEffect; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class SimpleCSP;
class GecodeRPGLayer;
class RPGIndex;
class GecodeCSPVariableTranslator;

//! A common baseclass for novelty constraints
class NoveltyConstraint {
public:
	virtual ~NoveltyConstraint() {}
	virtual void post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const = 0;
	
	//! Creates a suitable novelty constraint (strong if possible, weak if not) from a set of action preconditions and effects
	static NoveltyConstraint* createFromEffects(GecodeCSPVariableTranslator& translator, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects);
};


//! A WeakNoveltyConstraint object is in charge of registering the necessary variables and posting the necessary constraints
//! for a RPG novelty constraint enforcing that at least one of the variables that are relevant for an action is taking
//! values from the subset of its RPG domain that only contains the values that were achieved on the last RPG layer.
//! This effectively allows us to discard solutions to the CSP that have already been explored in previous layers.
class WeakNoveltyConstraint : public NoveltyConstraint {
public:
	
	static WeakNoveltyConstraint* create(GecodeCSPVariableTranslator& translator, const fs::Formula* conditions, const std::vector<const fs::ActionEffect*>& effects);
	
	//! Register the necessary variables for a novelty constraint to be posted upon two sets of variables, those
	//! that are directly present as relevant state variables ('direct'), and those that are present as part of
	//! a nested fluent ('derived').
	WeakNoveltyConstraint(GecodeCSPVariableTranslator& translator, const std::set<VariableIdx>& variables, const std::vector<unsigned> symbols);
	
	//! Post the novelty constraint to the given CSP and with the delta values given by 'layer'
	void post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const;


protected:
	//! contains a list of size-3 tuples, where each tuple contains:
	//! - ID of the planning variable
	//! - Index of the corresponding CSP variable
	//! - Index of the corresponding boolean CSP reification variable
	std::vector<std::tuple<VariableIdx, unsigned, unsigned>> _variables;
	
	//! A list of the logical symbols (e.g. "clear", "loc", etc.) that are relevant to the satisfiability of the CSP-
	std::vector<unsigned> _symbols;
};

//! A strong novelty constraint works only for actions, ans basically states that in any solution
//! at least one of the effects of the action produces a value which is new with respect to the state
//! variable where it will be assigned.
//! Thus, when the LHS of some effect has nested fluents, we cannot apply this type of novelty constraint,
//! since we cannot know in advance which is the affected state variable.
class StrongNoveltyConstraint : public NoveltyConstraint {
public:
	//! Returns true iff the constraint is applicable to the set of given effects
	//! The constraint is applicable if none of the effects' LHS contains a nested fluent
	static bool applicable(const std::vector<const fs::ActionEffect*>& effects);
	
	//! Create the constraint and register the necessary variables for the constraint
	StrongNoveltyConstraint(GecodeCSPVariableTranslator& translator, const std::vector<const fs::ActionEffect*>& effects);
	
	//! Post the novelty constraint to the given CSP and with the delta values given by 'layer'
	void post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const;
	
protected:
	//! contains a list of size-3 tuples, where each tuple contains:
	//! - ID of the LHS planning variable
	//! - Index of the corresponding RHS CSP variable
	//! - Index of the corresponding boolean CSP reification variable	
	std::vector<std::tuple<VariableIdx, unsigned, unsigned>> _variables;
};


//! 
class EffectNoveltyConstraint {
public:
	//! Returns true iff the constraint is applicable to the given effect, i.e. if the effect head is not nested.
	static bool applicable(const fs::ActionEffect* effect);
	
	//! Create the constraint and register the necessary variables for the constraint
	EffectNoveltyConstraint(GecodeCSPVariableTranslator& translator, const fs::ActionEffect* effect);
	
	//! Post the novelty constraint to the given CSP and with the delta values given by 'layer'
	void post_constraint(SimpleCSP& csp, const RPGIndex& rpg) const;
	
protected:
	//! contains a size-3 tuple with:
	//! - ID of the LHS planning variable
	//! - Index of the corresponding RHS CSP variable
	//! - Index of the corresponding boolean CSP reification variable	
	std::tuple<VariableIdx, unsigned, unsigned> _variable;
};
} } // namespaces


