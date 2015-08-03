
#pragma once

#include <languages/fstrips/language.hxx>

#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class GecodeCSPVariableTranslator; class SimpleCSP;

//! Utility class to store information related to temporary variables.
class TranslationData {
public:
	TranslationData(int id, int type) : temp_variable_id(id), temp_variable_type(type) {}
	
	unsigned getVariableId() const { return temp_variable_id; }
	unsigned getVariableType() const { return temp_variable_type; }

protected:
	unsigned temp_variable_id;
	unsigned temp_variable_type;
};

//! Abstract constraint translator
class GecodeBaseTranslator {
public:
	GecodeBaseTranslator(SimpleCSP& csp, GecodeCSPVariableTranslator& translator);
	virtual ~GecodeBaseTranslator() {}

	//! The translator can optionally register any number of (probably temporary) CSP variables.
	virtual void registerVariables(Gecode::IntVarArgs& variables) = 0;
	
	//! The translator can register any number of CSP constraints
	virtual void registerConstraints() const = 0;
	
protected:
	//! A helper data structure to help translate language relational symbols to gecode relational operators
	const static std::map<fs::RelationalFormula::Symbol, Gecode::IntRelType> symbol_to_gecode;
	
	//! A helper function to recover the gecode relational operator from a formula
	static Gecode::IntRelType gecode_symbol(fs::RelationalFormula::cptr formula);
	
	//! A helper data structure to map gecode operators with their inverse.
	const static std::map<Gecode::IntRelType, Gecode::IntRelType> operator_inversions;
	
	//! A helper to invert (e.g. from < to >=) a given gecode relational operator
	static Gecode::IntRelType invert_operator(Gecode::IntRelType op);
	
	
	
	SimpleCSP& _csp;
	
	GecodeCSPVariableTranslator& _translator;
	
	//! A map of already-processed terms and their assigned temporary data.
	std::map<const fs::Term*, TranslationData> _temp_variables;

	
	// Variable registration methods
	void registerTempVariables(const fs::Term::cptr term, Gecode::IntVarArgs& variables);
	void registerTempVariables(const std::vector<fs::Term::cptr>& terms, Gecode::IntVarArgs& variables);
	void registerTempVariables(const fs::AtomicFormula::cptr condition, Gecode::IntVarArgs& variables);
	void registerTempVariables(const fs::RelationalFormula::cptr condition, Gecode::IntVarArgs& variables);
	void registerTempVariables(const std::vector<fs::AtomicFormula::cptr>& conditions, Gecode::IntVarArgs& variables);
	
	// Constraint registration methods
	void registerConstraints(const fs::Term::cptr term) const;
	void registerConstraints(const std::vector<fs::Term::cptr>& terms) const;
	void registerConstraints(const fs::AtomicFormula::cptr condition) const;
	void registerConstraints(const fs::RelationalFormula::cptr condition) const;
	void registerConstraints(const std::vector<fs::AtomicFormula::cptr>& conditions) const;
	
	//! Conditionally register the constraints associated with certain type of terms
	void registerConstraintsMaybe(const fs::FluentHeadedNestedTerm::cptr fluent, unsigned output_var_id) const;
	void registerConstraintsMaybe(const fs::StaticHeadedNestedTerm::cptr _static, unsigned output_var_id) const;
};

} } // namespaces
