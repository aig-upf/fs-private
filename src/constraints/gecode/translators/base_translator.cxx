
#include <constraints/gecode/translators/base_translator.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/direct/compiled.hxx>
#include <problem.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace gecode {
	
	typedef fs::RelationalFormula::Symbol AFSymbol;
	const std::map<AFSymbol, Gecode::IntRelType> GecodeBaseTranslator::symbol_to_gecode = {
		{AFSymbol::EQ, Gecode::IRT_EQ}, {AFSymbol::NEQ, Gecode::IRT_NQ}, {AFSymbol::LT, Gecode::IRT_LE},
		{AFSymbol::LEQ, Gecode::IRT_LQ}, {AFSymbol::GT, Gecode::IRT_GR}, {AFSymbol::GEQ, Gecode::IRT_GQ}
	};
	
	Gecode::IntRelType GecodeBaseTranslator::gecode_symbol(fs::RelationalFormula::cptr formula) { return symbol_to_gecode.at(formula->symbol()); }
	
	
	const std::map<Gecode::IntRelType, Gecode::IntRelType> GecodeBaseTranslator::operator_inversions = {
		{Gecode::IRT_EQ, Gecode::IRT_EQ}, {Gecode::IRT_NQ, Gecode::IRT_NQ}, {Gecode::IRT_LE, Gecode::IRT_GQ},
		{Gecode::IRT_LQ, Gecode::IRT_GR}, {Gecode::IRT_GR, Gecode::IRT_LQ}, {Gecode::IRT_GQ, Gecode::IRT_LE}
	};
	
	Gecode::IntRelType GecodeBaseTranslator::invert_operator(Gecode::IntRelType op) {
		return operator_inversions.at(op);
	}
	
	
	GecodeBaseTranslator::GecodeBaseTranslator(SimpleCSP& csp, GecodeCSPVariableTranslator& translator) 
		: _csp(csp), _translator(translator)
	{}
	
	
	void GecodeBaseTranslator::registerTempVariables(const fs::Term::cptr term, Gecode::IntVarArgs& variables) {
		const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
		
		// Only nested terms can produce temporary variables
		auto _nested = dynamic_cast<NestedTerm::cptr>(term);
		if (!_nested) return;
		
		// If the subterm occurs somewhere else in the action, it might have already been parsed and register,
		// in which case we do NOT want to register it again
		auto it = _temp_variables.find(_nested);
		if (it != _temp_variables.end()) return;
		
		// We first parse and register the subterms recursively
		registerTempVariables(_nested->getSubterms(), variables);
		
		// And now register the variables from the current term
		unsigned id = _temp_variables.size(); // The ID of the temporary variable is implicitly given by the number of previous variables
		TypeIdx type = info.getFunctionData(_nested->getSymbolId()).getCodomainType();
		_temp_variables.insert(std::make_pair(_nested, TranslationData(id, type)));
		
		Helper::registerTemporaryVariable(_csp, id, variables, _translator, type);
	}
	
	void GecodeBaseTranslator::registerTempVariables(const std::vector<fs::Term::cptr>& terms, Gecode::IntVarArgs& variables) {
		for (const auto term:terms) registerTempVariables(term, variables);
	}
	
	void GecodeBaseTranslator::registerTempVariables(const fs::AtomicFormula::cptr condition, Gecode::IntVarArgs& variables) {
		if (auto relational = dynamic_cast<fs::RelationalFormula::cptr>(condition)) registerTempVariables(relational, variables);
		// Else, it must be a built-in, or external condition??
		throw std::runtime_error("Unimplemented!");
	}
	
	void GecodeBaseTranslator::registerTempVariables(const fs::RelationalFormula::cptr condition, Gecode::IntVarArgs& variables) {
		registerTempVariables(condition->lhs(), variables);
		registerTempVariables(condition->rhs(), variables);
	}
	
	void GecodeBaseTranslator::registerTempVariables(const std::vector<fs::AtomicFormula::cptr>& conditions, Gecode::IntVarArgs& variables) {
		for (const auto condition:conditions) registerTempVariables(condition, variables);
	}
	
	
	void GecodeBaseTranslator::registerConstraints(const std::vector<fs::AtomicFormula::cptr>& conditions) const {
		for (const auto condition:conditions) registerConstraints(condition);
	}
	
	
	void GecodeBaseTranslator::registerConstraints(const fs::AtomicFormula::cptr condition) const {
		if (auto relational = dynamic_cast<fs::RelationalFormula::cptr>(condition)) return registerConstraints(relational);
		// Else, it must be a built-in, or external condition??
		throw std::runtime_error("Unimplemented!");
	}
	
	void GecodeBaseTranslator::registerConstraints(const fs::RelationalFormula::cptr condition) const {
		
		// Register possible nested constraints recursively:
		registerConstraints(condition->lhs());
		registerConstraints(condition->rhs());
		
		
		// An extremely ugly case-based analysis of the different types of subterms that we can encounter
		
		auto lhs_nested = dynamic_cast<NestedTerm::cptr>(condition->lhs());
		auto rhs_nested = dynamic_cast<NestedTerm::cptr>(condition->rhs());
		auto lhs_var = dynamic_cast<StateVariable::cptr>(condition->lhs());
		auto rhs_var = dynamic_cast<StateVariable::cptr>(condition->rhs());
		auto lhs_const = dynamic_cast<Constant::cptr>(condition->lhs());
		auto rhs_const = dynamic_cast<Constant::cptr>(condition->rhs());
		
		
		// Case 1
		if (lhs_const && rhs_const) { // A comparison between two constants... shouldn't get to this point
			throw std::runtime_error("Comparison between two constants");
		}

		// Case 2
		if (lhs_var && rhs_var) { // X = Y
			auto& lhs_gec_var = _translator.resolveVariable(_csp, lhs_var->getValue(), CSPVariableType::Input);
			auto& rhs_gec_var = _translator.resolveVariable(_csp, rhs_var->getValue(), CSPVariableType::Input);
			Gecode::rel(_csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
		}
		
		// Case 3
		else if (lhs_var && rhs_const) { // X = c
			auto& lhs_gec_var = _translator.resolveVariable(_csp, lhs_var->getValue(), CSPVariableType::Input);
			Gecode::rel(_csp, lhs_gec_var, gecode_symbol(condition), rhs_const->getValue());
		}
		 
		// Case 4
		else if (lhs_const && rhs_var) { // c = X
			auto& rhs_gec_var = _translator.resolveVariable(_csp, rhs_var->getValue(), CSPVariableType::Input);
			auto inverted = invert_operator(gecode_symbol(condition));
			Gecode::rel(_csp, rhs_gec_var, inverted, lhs_const->getValue());
		}
		
		// Case 5
		else if (lhs_nested && rhs_nested) { // f(X) = g(Y)
			unsigned lhs_id = _temp_variables.at(lhs_nested).getVariableId();
			auto& lhs_gec_var = _translator.resolveVariable(_csp, lhs_id, CSPVariableType::Temporary);
			unsigned rhs_id = _temp_variables.at(rhs_nested).getVariableId();
			auto& rhs_gec_var = _translator.resolveVariable(_csp, rhs_id, CSPVariableType::Temporary);
			Gecode::rel(_csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
		}
		
		// Case 6
		else if (lhs_nested && rhs_var) { // f(X) = Y
			unsigned lhs_id = _temp_variables.at(lhs_nested).getVariableId();
			auto& lhs_gec_var = _translator.resolveVariable(_csp, lhs_id, CSPVariableType::Temporary);
			auto& rhs_gec_var = _translator.resolveVariable(_csp, rhs_var->getValue(), CSPVariableType::Input);
			Gecode::rel(_csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
		}
		
		// Case 7
		else if (lhs_nested && rhs_const) { // f(X) = c
			unsigned lhs_id = _temp_variables.at(lhs_nested).getVariableId();
			auto& lhs_gec_var = _translator.resolveVariable(_csp, lhs_id, CSPVariableType::Temporary);
			Gecode::rel(_csp, lhs_gec_var, gecode_symbol(condition), rhs_const->getValue());
		}
		
		// Case 8
		else if (lhs_var && rhs_nested) { // X = g(Y)
			auto& lhs_gec_var = _translator.resolveVariable(_csp, lhs_var->getValue(), CSPVariableType::Input);
			unsigned rhs_id = _temp_variables.at(rhs_nested).getVariableId();
			auto& rhs_gec_var = _translator.resolveVariable(_csp, rhs_id, CSPVariableType::Temporary);
			Gecode::rel(_csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
		}
		
		// Case 9
		else if (lhs_const && rhs_nested) { // f(X) = Y
			unsigned rhs_id = _temp_variables.at(rhs_nested).getVariableId();
			auto& rhs_gec_var = _translator.resolveVariable(_csp, rhs_id, CSPVariableType::Temporary);
			auto inverted = invert_operator(gecode_symbol(condition));
			Gecode::rel(_csp, rhs_gec_var, inverted, lhs_const->getValue());			
		}
	}
	
	void GecodeBaseTranslator::registerConstraints(const std::vector<fs::Term::cptr>& terms) const {
		for (const auto term:terms) registerConstraints(term);
	}
	
	void GecodeBaseTranslator::registerConstraints(const fs::Term::cptr term) const {
		auto it = _temp_variables.find(term);
		if (it == _temp_variables.end()) return; // There's no nested subterm to register any constraint with
		
		unsigned tmp_id = it->second.getVariableId();
		
		auto _nested = dynamic_cast<NestedTerm::cptr>(term);
		assert(_nested); // We necessarily have a nested term
		registerConstraints(_nested->getSubterms()); // Register recursively possible subterms
		
		registerConstraintsMaybe(dynamic_cast<StaticHeadedNestedTerm::cptr>(term), tmp_id);
		registerConstraintsMaybe(dynamic_cast<FluentHeadedNestedTerm::cptr>(term), tmp_id);
	}
	
	// We use an element constraint to model fluent functionals
	void GecodeBaseTranslator::registerConstraintsMaybe(const fs::FluentHeadedNestedTerm::cptr fluent, unsigned output_var_id) const {
		if (!fluent) return;
		
		const std::vector<fs::Term::cptr>& subterms = fluent->getSubterms();
		if (subterms.size() > 1) throw std::runtime_error("Nested subterms of arity > 1 not yet implemented");
		assert(subterms.size() == 1); // Cannot be 0, or we'd have instead a StateVariable term
		fs::Term::cptr st = subterms[0];
		
		IntVar index_var;
		auto it = _temp_variables.find(st);
		if (it != _temp_variables.end()) {
			index_var = _translator.resolveVariable(_csp, it->second.getVariableId(), CSPVariableType::Temporary);
		} else {
			auto _var = dynamic_cast<StateVariable::cptr>(st);
			assert(_var);
			index_var = _translator.resolveVariable(_csp, _var->getValue(), CSPVariableType::Temporary);
		}
		
		Gecode::IntVarArgs function_vars = _translator.resolveFunction(_csp, fluent->getSymbolId(), CSPVariableType::Input);
		auto output_var = _translator.resolveFunction(_csp, output_var_id, CSPVariableType::Temporary);
		
		assert(false); // TODO - REGISTER the full element constraint, with reindexing.
		
// 		Gecode::element(_csp, function_vars, index_var, output_var);
	}
	
	// To model staticly-headed nested terms, ATM we compile them into an extensional constraint.
	void GecodeBaseTranslator::registerConstraintsMaybe(const fs::StaticHeadedNestedTerm::cptr _static, unsigned output_var_id) const {
		if (!_static) return;
		
		VariableIdxVector scope = _static->computeScope();
		Gecode::TupleSet extension = Helper::extensionalize(_static, scope);

		
		// TODO - This is incorrect - we need to compile static fluents differently
		// We need to use both state variables and temporary variables
		assert(0); 
		
		IntVarArgs ordered_variables;
		
		auto output_var = _translator.resolveFunction(_csp, output_var_id, CSPVariableType::Temporary);
		
		// TODO - Add additional variables corresponding to temporaries...
		
		ordered_variables << output_var;
		
		Gecode::extensional(_csp, ordered_variables, extension); // Order matters - variable order must be the same than in the tupleset
	}	
	

} } // namespaces
