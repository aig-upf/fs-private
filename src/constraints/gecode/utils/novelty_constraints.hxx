

#pragma once

#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <boost/functional/hash.hpp>
#include <gecode/int.hh>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

//! A common baseclass for novelty constraints
class NoveltyConstraint {
public:
	virtual ~NoveltyConstraint() {}
	virtual void post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const = 0;
	
	//! Creates a suitable novelty constraint (strong if possible, weak if not) from a set of action preconditions and effects
	static NoveltyConstraint* createFromEffects(GecodeCSPVariableTranslator& translator, const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects);
};


//! A WeakNoveltyConstraint object is in charge of registering the necessary variables and posting the necessary constraints
//! for a RPG novelty constraint enforcing that at least one of the variables that are relevant for an action is taking
//! values from the subset of its RPG domain that only contains the values that were achieved on the last RPG layer.
//! This effectively allows us to discard solutions to the CSP that have already been explored in previous layers.
class WeakNoveltyConstraint : public NoveltyConstraint {
public:
	
	static WeakNoveltyConstraint* create(GecodeCSPVariableTranslator& translator, const Formula::cptr conditions, const std::vector<fs::ActionEffect::cptr>& effects) {
		std::set<VariableIdx> relevant;

		ScopeUtils::computeVariables(conditions, relevant, relevant);
		
		for (auto effect:effects) {
			ScopeUtils::computeVariables(effect->rhs(), relevant, relevant);
		}
		
		return new WeakNoveltyConstraint(translator, relevant);
	}	
	
	//! Register the necessary variables for a novelty constraint to be posted upon two sets of variables, those
	//! that are directly present as relevant state variables ('direct'), and those that are present as part of
	//! a nested fluent ('derived').
	WeakNoveltyConstraint(GecodeCSPVariableTranslator& translator, const std::set<VariableIdx>& relevant) {
		for (VariableIdx variable:relevant) {
			// We assume here that the state variable CSP var has already been registered
			unsigned csp_var_id = translator.resolveInputVariableIndex(variable);
			unsigned reified_id = translator.create_bool_variable();
			_variables.push_back(std::make_tuple(variable, csp_var_id, reified_id));
		}
	}
	
	//! Post the novelty constraint to the given CSP and with the delta values given by 'layer'
	void post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
		if (_variables.empty()) return;
		
		Gecode::BoolVarArgs delta_reification_variables;
		
		for (const auto& element:_variables) {
			VariableIdx variable = std::get<0>(element);
			unsigned csp_variable_id = std::get<1>(element);
			unsigned reified_variable_id = std::get<2>(element);
			
			const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];
			auto& novelty_reification_variable = csp._boolvars[reified_variable_id];
			delta_reification_variables << novelty_reification_variable;
			Gecode::dom(csp, csp_variable, layer.get_delta(variable), novelty_reification_variable);  // X_i in delta(i) (i.e. X_i is new)
		}
		
		// Now post the global novelty constraint OR: X1 is new, or X2 is new, or...
		Gecode::rel(csp, Gecode::BOT_OR, delta_reification_variables, 1);
	}


protected:
	//! contains a list of size-3 tuples, where each tuple contains:
	//! - ID of the planning variable
	//! - Index of the corresponding CSP variable
	//! - Index of the corresponding boolean CSP reification variable
	std::vector<std::tuple<VariableIdx, unsigned, unsigned>> _variables;
};


class StrongNoveltyConstraint : public NoveltyConstraint {
public:
	//! Returns true iff the constraint is applicable to the set of given effects
	//! The constraint is applicable if none of the effects' LHS contains a nested fluent
	static bool applicable(const std::vector<fs::ActionEffect::cptr>& effects) {
		for (const auto effect:effects) {
			if (!effect->lhs()->flat()) return false;
		}
		return true;
	}
	
	//! Create the constraint and register the necessary variables for the constraint
	StrongNoveltyConstraint(GecodeCSPVariableTranslator& translator, const std::vector<fs::ActionEffect::cptr>& effects)  {
		assert(applicable(effects));
		for (const auto effect:effects) {
			auto variable = dynamic_cast<fs::StateVariable::cptr>(effect->lhs());
			assert(variable);
			unsigned csp_var_id = translator.resolveVariableIndex(effect->rhs(), CSPVariableType::Input);
			unsigned reified_id = translator.create_bool_variable();
			_variables.push_back(std::make_tuple(variable->getValue(), csp_var_id, reified_id));
		}
	}
	
	
	//! Post the novelty constraint to the given CSP and with the delta values given by 'layer'
	void post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
		if (_variables.empty()) return;
		
		Gecode::BoolVarArgs reification_variables;
		for (const auto& element:_variables) {
			reification_variables << post_individual_constraint(csp, layer, element);
		}
		
		// Now post the strong novelty constraint: NOT (w_1 is OLD and ... and w_n is OLD),
		// i.e. at least some of the LHS is new wrt to the already-accumulated values of the LHS
		Gecode::rel(csp, Gecode::BOT_AND, reification_variables, 0);
	}
	
	Gecode::BoolVar& post_individual_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer, const std::tuple<VariableIdx, unsigned, unsigned>& element) const {
		VariableIdx variable = std::get<0>(element);
		unsigned csp_variable_id = std::get<1>(element);
		unsigned reified_variable_id = std::get<2>(element);
		
		const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];
		auto& reification_variable = csp._boolvars[reified_variable_id];
		Gecode::dom(csp, csp_variable, layer.get_domain(variable), reification_variable);	
		return reification_variable;
	}


protected:
	//! contains a list of size-3 tuples, where each tuple contains:
	//! - ID of the LHS planning variable
	//! - Index of the corresponding RHS CSP variable
	//! - Index of the corresponding boolean CSP reification variable	
	std::vector<std::tuple<VariableIdx, unsigned, unsigned>> _variables;
};


} } // namespaces


