
#pragma once

#include <typeindex>
#include <typeinfo>

#include <fs0_types.hxx>
#include <languages/fstrips/language.hxx>
#include <constraints/direct/constraint.hxx>
#include <constraints/gecode/translators/component_translator.hxx>


namespace fs = fs0::language::fstrips;

namespace fs0 { namespace print { class logical_registry; } }

namespace fs0 {

//! The LogicalComponentRegistry is a singleton object that provides access to a number of classes and methods
//! that know how to translate from and into FSTRIPS logical elements (terms, formulae).
//! At the moment this mainly includes creating terms and formulae from their symbol names and, most relevantly,
//! providing access to Direct/Gecode translator that know how to transform those particular logical elements
//! into operational constraints.
class LogicalComponentRegistry {
public:
	~LogicalComponentRegistry();
	
	//!
	typedef std::function<fs::AtomicFormula::cptr(const std::vector<fs::Term::cptr>&)> FormulaCreator;
	
	//!
	typedef std::function<DirectConstraint::cptr(const fs::AtomicFormula&)> DirectFormulaTranslator;
	
	static LogicalComponentRegistry& instance();
	
	fs::AtomicFormula::cptr instantiate_formula(const std::string symbol, const std::vector<fs::Term::cptr>& subterms) const;
	
	DirectConstraint::cptr instantiate_direct_constraint(const fs::AtomicFormula& formula) const;
	
	gecode::TermTranslator::cptr getGecodeTranslator(const fs::Term& term) const;
	gecode::AtomicFormulaTranslator::cptr getGecodeTranslator(const fs::AtomicFormula& formula) const;
	
	void add(const std::string& symbol, const FormulaCreator& creator);
	
	void add(const std::type_info& type, const DirectFormulaTranslator& creator);
	
	void add(const std::type_info& type, const gecode::TermTranslator::cptr translator);
	void add(const std::type_info& type, const gecode::AtomicFormulaTranslator::cptr translator);
	
	friend class print::logical_registry; // Grant access to the corresponding printer class
	
protected:
	LogicalComponentRegistry();
	
	std::map<std::string, FormulaCreator> _formula_creators;
	
	typedef std::unordered_map<std::type_index, DirectFormulaTranslator> DirectTranslatorsTable;
	DirectTranslatorsTable _direct_formula_translators;
	
	
	std::unordered_map<std::type_index, gecode::TermTranslator::cptr> _gecode_term_translators;
	std::unordered_map<std::type_index, gecode::AtomicFormulaTranslator::cptr> _gecode_formula_translators;
};

} // namespaces
