

#pragma once

#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <boost/functional/hash.hpp>
#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

//! A NoveltyConstraint object is in charge of registering the necessary variables and posting the necessary constraints
//! for a RPG novelty constraint enforcing that at least one of the variables that are relevant for an action is taking
//! values from the subset of its RPG domain that only contains the values that were achieved on the last RPG layer.
//! This effectively allows us to discard solutions to the CSP that have already been explored in previous layers.
class NoveltyConstraint {
public:
	//! The constructor solely requires a reference to the CSP translator object
	NoveltyConstraint(GecodeCSPVariableTranslator& translator) : _translator(translator) {}
	
	//! Register the necessary variables for a novelty constraint to be posted upon two sets of variables, those
	//! that are directly present as relevant state variables ('direct'), and those that are present as part of
	//! a nested fluent ('derived').
	void register_variables(const std::set<VariableIdx>& direct, const std::set<VariableIdx>& derived) {
		for (VariableIdx variable:direct) {
			// We assume here that the state variable CSP var has already been registered
			unsigned csp_var_id = _translator.resolveInputVariableIndex(variable);
			unsigned reified_id = _translator.create_bool_variable();
			_variables.push_back(std::make_tuple(variable, csp_var_id, reified_id));
		}
		
		for (VariableIdx variable:derived) {
			unsigned csp_var_id = _translator.resolveDerivedVariableIndex(variable);
			unsigned reified_id = _translator.create_bool_variable();
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
			Gecode::dom(csp, csp_variable, layer.get_delta(variable), novelty_reification_variable);
		}
		
		// Now post the global novelty constraint OR: X1 is new, or X2 is new, or...
		Gecode::rel(csp, Gecode::BOT_OR, delta_reification_variables, 1);
	}


protected:
	// A reference to the CSP translator
	GecodeCSPVariableTranslator& _translator;
	
	//! contains a list of size-3 tuples, where each tuple contains:
	//! - ID of the planning variable
	//! - Index of the corresponding CSP variable
	//! - Index of the corresponding boolean CSP reification variable
	std::vector<std::tuple<VariableIdx, unsigned, unsigned>> _variables;
};


} } // namespaces


