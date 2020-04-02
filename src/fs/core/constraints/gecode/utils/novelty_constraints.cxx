

#include <fs/core/constraints/gecode/utils/novelty_constraints.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/languages/fstrips/scopes.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>
#include <fs/core/constraints/gecode/gecode_space.hxx>
#include <fs/core/heuristics/relaxed_plan/rpg_index.hxx>

namespace fs0 { namespace gecode {
	
NoveltyConstraint* NoveltyConstraint::createFromEffects(CSPTranslator& translator, const fs::Formula* precondition, const std::vector<const fs::ActionEffect*>& effects) {
	if (StrongNoveltyConstraint::applicable(effects)) {
		return new StrongNoveltyConstraint(translator, effects);
	} else {
		// Weak novelty constraints are only applicable for plain conjunctions or existentially quantified conjunctions
		return nullptr;
// 		return WeakNoveltyConstraint::create(translator, precondition, effects);
	}
}

// TODO - Weak Novelty constraints ATM disabled, as it is not sure it pays off to compute and keep the whole domain deltas for such a weak constraint.
/*
WeakNoveltyConstraint* WeakNoveltyConstraint::create(CSPTranslator& translator, const fs::Formula* conditions, const std::vector<const fs::ActionEffect*>& effects) {
	std::set<VariableIdx> variables;
	std::set<unsigned> symbols;

	fs::ScopeUtils::computeRelevantElements(conditions, variables, symbols);
	
	for (auto effect:effects) {
		fs::ScopeUtils::computeRelevantElements(effect->rhs(), variables, symbols);
	}
	
	return new WeakNoveltyConstraint(translator, variables, std::vector<unsigned>(symbols.cbegin(), symbols.cend()));
}

WeakNoveltyConstraint::WeakNoveltyConstraint(CSPTranslator& translator, const std::set<VariableIdx>& variables, const std::vector<unsigned> symbols) 
	: _symbols(symbols)
{
	for (VariableIdx variable:variables) {
		// We assume here that the state variable CSP var has already been registered
		unsigned csp_var_id = translator.resolveInputVariableIndex(variable);
		unsigned reified_id = translator.create_bool_variable();
		_variables.push_back(std::make_tuple(variable, csp_var_id, reified_id));
	}
}

void WeakNoveltyConstraint::post_constraint(GecodeSpace& csp, const RPGIndex& layer) const {
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
*/

//! Returns true iff the constraint is applicable to the set of given effects
//! The constraint is applicable if none of the effects' LHS contains a nested fluent
bool StrongNoveltyConstraint::applicable(const std::vector<const fs::ActionEffect*>& effects) {
	for (const auto effect:effects) {
		if (!fs::flat(*effect->lhs())) return false;
	}
	return true;
}
	
StrongNoveltyConstraint::StrongNoveltyConstraint(CSPTranslator& translator, const std::vector<const fs::ActionEffect*>& effects)  {
	assert(applicable(effects));
	for (const auto effect:effects) {
		auto variable = dynamic_cast<const fs::StateVariable*>(effect->lhs());
		assert(variable);
		unsigned csp_var_id = translator.resolveVariableIndex(effect->rhs());
		unsigned reified_id = translator.create_bool_variable();
		_variables.push_back(std::make_tuple(variable->getValue(), csp_var_id, reified_id));
	}
}

//! A private helper
Gecode::BoolVar& post_individual_constraint(FSGecodeSpace& csp, const RPGIndex& layer, const std::tuple<VariableIdx, unsigned, unsigned>& element) {
	VariableIdx variable = std::get<0>(element);
	unsigned csp_variable_id = std::get<1>(element);
	unsigned reified_variable_id = std::get<2>(element);
	
	const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];
	auto& reification_variable = csp._boolvars[reified_variable_id];
	Gecode::dom(csp, csp_variable, layer.get_domain(variable), reification_variable);
	return reification_variable;
}

void StrongNoveltyConstraint::post_constraint(FSGecodeSpace& csp, const RPGIndex& layer) const {
	if (_variables.empty()) return;
	
	Gecode::BoolVarArgs reification_variables;
	for (const auto& element:_variables) {
		reification_variables << post_individual_constraint(csp, layer, element);
	}
	
	// Now post the strong novelty constraint: NOT (w_1 is OLD and ... and w_n is OLD),
	// i.e. at least some of the LHS is new wrt to the already-accumulated values of the LHS
	Gecode::rel(csp, Gecode::BOT_AND, reification_variables, 0);
}


bool EffectNoveltyConstraint::applicable(const fs::ActionEffect* effect) { return fs::flat(*effect->lhs()); }

EffectNoveltyConstraint::EffectNoveltyConstraint(CSPTranslator& translator, const fs::ActionEffect* effect)  {
	assert(applicable(effect));
	auto variable = dynamic_cast<const fs::StateVariable*>(effect->lhs());
	assert(variable);
	unsigned csp_var_id = translator.resolveVariableIndex(effect->rhs());
	unsigned reified_id = translator.create_bool_variable();
	_variable = std::make_tuple(variable->getValue(), csp_var_id, reified_id);
}

void EffectNoveltyConstraint::post_constraint(FSGecodeSpace& csp, const RPGIndex& layer) const {
	VariableIdx variable = std::get<0>(_variable);
	unsigned csp_variable_id = std::get<1>(_variable);
	unsigned reified_variable_id = std::get<2>(_variable);
	
	const Gecode::IntVar& csp_variable = csp._intvars[csp_variable_id];
	const Gecode::BoolVar& reification_variable = csp._boolvars[reified_variable_id];
	
	// Now post the effect novelty constraint: NOT w is OLD,
	// i.e. the LHS is new wrt to the already-accumulated values of the LHS
	Gecode::dom(csp, csp_variable, layer.get_domain(variable), reification_variable);
	Gecode::rel(csp, reification_variable, Gecode::IRT_EQ, 0);
}

} } // namespaces
