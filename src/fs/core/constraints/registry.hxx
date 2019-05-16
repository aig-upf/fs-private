
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/printers/helper.hxx>
#include <fs/core/languages/fstrips/language_fwd.hxx>

#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <sstream>


namespace fs0 { namespace print { class logical_registry; } }

namespace fs0 {

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

	//! Add a formula creator for formulae with the given symbol to the registry
	void addFormulaCreator(const std::string& symbol, const FormulaCreator& creator);

	//! Add a formula creator for formulae with the given symbol to the registry
	void addTermCreator(const std::string& symbol, const TermCreator& creator);

	//! Add an effect creator associated with the given symbol to the registry
	void addEffectCreator( const std::string& symbo, const EffectCreator& creator );


	const fs::AtomicFormula* instantiate_formula(const std::string& symbol, const std::vector<const fs::Term*>& subterms) const;

	const fs::Term* instantiate_term(const std::string& symbol, const std::vector<const fs::Term*>& subterms) const;

	fs::ProceduralEffect* instantiate_effect( const std::string& symbol ) const;

	friend class print::logical_registry; // Grant access to the corresponding printer class

protected:
	static std::unique_ptr<LogicalComponentRegistry> _instance;

	void registerLogicalElementCreators();

	std::map<std::string, FormulaCreator> _formula_creators;

	std::map<std::string, TermCreator> _term_creators;

	std::map<std::string, EffectCreator> _effect_creators;
};


} // namespaces
