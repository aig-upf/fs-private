
#pragma once

#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>
#include <constraints/direct/constraint.hxx>
#include <utils/typeinfo_helper.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {

class ExternalComponentRepository {
public:
	//!
	typedef std::function<fs::AtomicFormula::cptr(const std::vector<fs::Term::cptr>&)> FormulaCreator;
	
	//!
	typedef std::function<DirectConstraint::cptr(const fs::AtomicFormula&)> DirectFormulaTranslator;
	
	static ExternalComponentRepository& instance();
	
	fs::AtomicFormula::cptr instantiate_formula(const std::string symbol, const std::vector<fs::Term::cptr>& subterms) const;
	
	DirectConstraint::cptr instantiate_direct_constraint(const fs::AtomicFormula& formula) const;
	
	void add(const std::string& symbol, const FormulaCreator& creator);
	
	void add(TypeInfoRef type, const DirectFormulaTranslator& creator);
	
protected:
	ExternalComponentRepository();
	
	
	
	std::map<std::string, FormulaCreator> _formula_creators;
	
	typedef std::unordered_map<TypeInfoRef, DirectFormulaTranslator, TypeInfoHasher, TypeInfoEqualsTo > DirectTranslatorsTable;
	DirectTranslatorsTable _direct_formula_translators;
	
	
};

} // namespaces
