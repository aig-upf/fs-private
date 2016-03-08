

#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <constraints/gecode/rpg_layer.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <gecode/int.hh>
#include <constraints/gecode/simple_csp.hxx>
#include <utils/utils.hxx>

namespace fs0 { namespace gecode {
	
NoveltyConstraint* NoveltyConstraint::createFromEffects(GecodeCSPVariableTranslator& translator, const fs::Formula::cptr precondition, const std::vector<fs::ActionEffect::cptr>& effects) {
	if (StrongNoveltyConstraint::applicable(effects)) {
		return new StrongNoveltyConstraint(translator, effects);
	} else {
		// Weak novelty constraints are only applicable for plain conjunctions or existentially quantified conjunctions
		return WeakNoveltyConstraint::create(translator, precondition, effects);
	}
}


WeakNoveltyConstraint* WeakNoveltyConstraint::create(GecodeCSPVariableTranslator& translator, const fs::Formula::cptr conditions, const std::vector<fs::ActionEffect::cptr>& effects) {
	std::set<VariableIdx> variables;
	std::set<unsigned> symbols;

	fs::ScopeUtils::computeRelevantElements(conditions, variables, symbols);
	
	for (auto effect:effects) {
		fs::ScopeUtils::computeRelevantElements(effect->rhs(), variables, symbols);
	}
	
	return new WeakNoveltyConstraint(translator, variables, std::vector<unsigned>(symbols.cbegin(), symbols.cend()));
}

WeakNoveltyConstraint::WeakNoveltyConstraint(GecodeCSPVariableTranslator& translator, const std::set<VariableIdx>& variables, const std::vector<unsigned> symbols) 
	: _symbols(symbols)
{
	for (VariableIdx variable:variables) {
		// We assume here that the state variable CSP var has already been registered
		unsigned csp_var_id = translator.resolveInputVariableIndex(variable);
		unsigned reified_id = translator.create_bool_variable();
		_variables.push_back(std::make_tuple(variable, csp_var_id, reified_id));
	}
}

void WeakNoveltyConstraint::post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
	// If we have relevant predicative symbols, and the denotation of some of them has changed in the last
	// RPG layer, then we don't need to post the actual novelty constraint, since it might already be the case
	// that e.g. some atom that was false is now true, and thus we need to check all possible values, old and new,
	// of the state variables.
	const auto& modified = layer.get_modified_symbols();
	if (!_symbols.empty() && !Utils::empty_intersection(_symbols.cbegin(), _symbols.cend(), modified.begin(), modified.end())) {
		return;
	}
	
	// If there are no relevant state variables, then we are done.
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

//! Returns true iff the constraint is applicable to the set of given effects
//! The constraint is applicable if none of the effects' LHS contains a nested fluent
bool StrongNoveltyConstraint::applicable(const std::vector<fs::ActionEffect::cptr>& effects) {
	for (const auto effect:effects) {
		if (!effect->lhs()->flat()) return false;
	}
	return true;
}
	
StrongNoveltyConstraint::StrongNoveltyConstraint(GecodeCSPVariableTranslator& translator, const std::vector<fs::ActionEffect::cptr>& effects)  {
	assert(applicable(effects));
	for (const auto effect:effects) {
		auto variable = dynamic_cast<fs::StateVariable::cptr>(effect->lhs());
		assert(variable);
		unsigned csp_var_id = translator.resolveVariableIndex(effect->rhs(), CSPVariableType::Input);
		unsigned reified_id = translator.create_bool_variable();
		_variables.push_back(std::make_tuple(variable->getValue(), csp_var_id, reified_id));
	}
}

//! A private helper
Gecode::BoolVar& post_individual_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer, const std::tuple<VariableIdx, unsigned, unsigned>& element) {
	VariableIdx variable = std::get<0>(element);
	unsigned csp_variable_id = std::get<1>(element);
	unsigned reified_variable_id = std::get<2>(element);
	
	const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];
	auto& reification_variable = csp._boolvars[reified_variable_id];
	Gecode::dom(csp, csp_variable, layer.get_domain(variable), reification_variable);
	return reification_variable;
}

void StrongNoveltyConstraint::post_constraint(SimpleCSP& csp, const GecodeRPGLayer& layer) const {
	if (_variables.empty()) return;
	
	Gecode::BoolVarArgs reification_variables;
	for (const auto& element:_variables) {
		reification_variables << post_individual_constraint(csp, layer, element);
	}
	
	// Now post the strong novelty constraint: NOT (w_1 is OLD and ... and w_n is OLD),
	// i.e. at least some of the LHS is new wrt to the already-accumulated values of the LHS
	Gecode::rel(csp, Gecode::BOT_AND, reification_variables, 0);
}


} } // namespaces
