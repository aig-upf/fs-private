
#pragma once

#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <sstream>

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/printers/helper.hxx>

namespace fs0 { namespace language { namespace fstrips { class AtomicFormula; class Term; class ProceduralEffect; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace print { class logical_registry; } }

namespace fs0 { namespace gecode { class TermTranslator; class FormulaTranslator; } }

namespace fs0 {

// class DirectConstraint;
// class EffectTranslator;

//! The LogicalComponentRegistry is a singleton object that provides access to a number of classes and methods
//! that know how to translate from and into FSTRIPS logical elements (terms, formulae).
//! At the moment this mainly includes creating terms and formulae from their symbol names and, most relevantly,
//! providing access to Direct/Gecode translator that know how to transform those particular logical elements
//! into operational constraints.
class LogicalComponentRegistry {
public:
	//! Set the global singleton problem instance
	static LogicalComponentRegistry& set_instance(std::unique_ptr<LogicalComponentRegistry>&& registry) {
		assert(!_instance);
		_instance = std::move(registry);
		return *_instance;
	}

	static std::unique_ptr<LogicalComponentRegistry>&& claim_ownership() {
		return std::move(_instance);
	}

	//! Global singleton object accessor
	static LogicalComponentRegistry& instance() {
		assert(_instance);
		return *_instance;
	}
	LogicalComponentRegistry();
	~LogicalComponentRegistry();

	//!
	typedef std::function<const fs::AtomicFormula*(const std::vector<const fs::Term*>&)> FormulaCreator;

	//!
	typedef std::function<const fs::Term*(const std::vector<const fs::Term*>&)> TermCreator;

	//!
	typedef std::function<fs::ProceduralEffect*()> EffectCreator;

	//!
// 	typedef std::function<DirectConstraint*(const fs::AtomicFormula&)> DirectFormulaTranslator;

	//static LogicalComponentRegistry& instance();

	//! Add a formula creator for formulae with the given symbol to the registry
	void addFormulaCreator(const std::string& symbol, const FormulaCreator& creator);

	//! Add a formula creator for formulae with the given symbol to the registry
	void addTermCreator(const std::string& symbol, const TermCreator& creator);

	//! Add an effect creator associated with the given symbol to the registry
	void addEffectCreator( const std::string& symbo, const EffectCreator& creator );

	//! Add a Direct Formula translator of the given type to the registry
// 	void add(const std::type_info& type, const DirectFormulaTranslator& translator);

	//! Add a Direct effect translator for effects with RHS of the given type
// 	void add(const std::type_info& type, const EffectTranslator* translator);

	//! Add a Gecode Term translator for the given type to the registry
	void add(const std::type_info& type, const gecode::TermTranslator* translator);

	//! Add a Gecode Formula translator for the given type to the registry
	void add(const std::type_info& type, const gecode::FormulaTranslator* translator);

	const fs::AtomicFormula* instantiate_formula(const std::string& symbol, const std::vector<const fs::Term*>& subterms) const;

	const fs::Term* instantiate_term(const std::string& symbol, const std::vector<const fs::Term*>& subterms) const;

	fs::ProceduralEffect* instantiate_effect( const std::string& symbol ) const;

// 	DirectConstraint* instantiate_direct_constraint(const fs::AtomicFormula& formula) const;

// 	const EffectTranslator* getDirectEffectTranslator(const fs::Term& term) const;

	const gecode::TermTranslator* getGecodeTranslator(const fs::Term& term) const;

	const gecode::FormulaTranslator* getGecodeTranslator(const fs::AtomicFormula& formula) const;

	friend class print::logical_registry; // Grant access to the corresponding printer class

protected:
	static std::unique_ptr<LogicalComponentRegistry> _instance;

	void registerLogicalElementCreators();
// 	void registerDirectTranslators();
	void registerGecodeTranslators();

	std::map<std::string, FormulaCreator> _formula_creators;

	std::map<std::string, TermCreator> _term_creators;

	std::map<std::string, EffectCreator> _effect_creators;

// 	typedef std::unordered_map<std::type_index, DirectFormulaTranslator> DirectTranslatorsTable;
// 	DirectTranslatorsTable _direct_formula_translators;
// 	std::unordered_map<std::type_index, const EffectTranslator*> _direct_effect_translators;

	std::unordered_map<std::type_index, const gecode::TermTranslator*> _gecode_term_translators;
	std::unordered_map<std::type_index, const gecode::FormulaTranslator*> _gecode_formula_translators;
};


class UnregisteredGecodeTranslator : public std::runtime_error {
public:
	template <typename T>
	UnregisteredGecodeTranslator(const T& element) : std::runtime_error(message(element)) {}

	template <typename T>
	static std::string message(const T& element);
};

} // namespaces
