
#pragma once

#include <fs/core/languages/fstrips/terms.hxx>

#include <unordered_set>
#include <unordered_map>

#include <fs/core/fs_types.hxx>
#include <fs/core/atom.hxx>
#include <fs/core/constraints/gecode/gecode_csp.hxx>
#include <fs/core/constraints/gecode/utils/extensional_constraint.hxx>
#include <fs/core/constraints/gecode/utils/element_constraint.hxx>
#include <fs/core/constraints/gecode/csp_translator.hxx>


namespace fs0 { namespace language { namespace fstrips { class Term; class AtomicFormula; class FluentHeadedNestedTerm; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { class RPGData; class Binding; }

namespace fs0 { namespace gecode {

class StateBasedExtensionHandler;

//! The base interface class for all gecode CSP handlers
class BaseCSP {
public:
	BaseCSP(const AtomIndex& tuple_index, bool approximate);
	virtual ~BaseCSP() = default;
	BaseCSP(const BaseCSP&) = delete;
	BaseCSP(BaseCSP&&) = delete;
	BaseCSP& operator=(const BaseCSP&) = delete;
	BaseCSP& operator=(BaseCSP&&) = delete;
	
	//! Create a new action CSP constraint by the given RPG layer domains
	//! Ownership of the generated pointer belongs to the caller
	GecodeCSP* instantiate(const RPGIndex& graph) const;
	GecodeCSP* instantiate(const State& state, const StateBasedExtensionHandler& handler) const;
	GecodeCSP* instantiate_wo_novelty(const RPGIndex& graph) const;
	
	void update_csp(std::unique_ptr<GecodeCSP>&& csp);
	
	const CSPTranslator& getTranslator() const { return _translator; }

	static void registerTermVariables(const fs::Term* term, CSPTranslator& translator);
	static void registerTermConstraints(const fs::Term* term, CSPTranslator& translator);
	static void registerFormulaVariables(const fs::AtomicFormula* condition, CSPTranslator& translator);
	static void registerFormulaConstraints(const fs::AtomicFormula* condition, CSPTranslator& translator);
	
	bool failed() const { return _failed; }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const BaseCSP& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const { return print(os, *_base_csp); }
	std::ostream& print(std::ostream& os, const GecodeCSP& csp) const;
	
protected:
	//! The base Gecode CSP
	std::unique_ptr<GecodeCSP> _base_csp;
	
	//! Whether the underlying CSP gecode space has already been detected as failed.
	bool _failed;
	
	//! Whether to solve the CSPs completely or approximately
	bool _approximate;

	//! A translator to map planning variables with gecode variables
	CSPTranslator _translator;
	
	// All (distinct) FSTRIPS terms that participate in the CSP
	std::unordered_set<const fs::Term*> _all_terms;
	
	// All (distinct) FSTRIPS atomic formulas that participate in the CSP
	std::unordered_set<const fs::AtomicFormula*> _all_formulas;
	
	//! The Ids of the state variables that are relevant to some formula via a (predicative) atom.
	std::vector<AtomIdx> _necessary_tuples;
	
	const AtomIndex& _tuple_index;
	
	//!
	std::vector<ExtensionalConstraint> _extensional_constraints;
	
	//! An index from the actual term to the position of the translator in the vector '_nested_fluent_translators'
 	//! Note that we need to use the hash and equality specializations of the parent class Term pointer
 	std::unordered_map<const fs::FluentHeadedNestedTerm*, unsigned, std::hash<const fs::Term*>, std::equal_to<const fs::Term*>> _nested_fluent_translators_idx;
 	
	//! The set of nested fluent translators, one for each nested fluent in the set of terms modeled by this CSP
	std::vector<NestedFluentElementTranslator> _nested_fluent_translators;
	
	VariableCounter _counter;
	
	//! Index all terms and formulas appearing in the formula / actions which will be relevant to the CSP
	virtual void index() = 0;

	virtual void register_csp_variables();

	void register_csp_constraints();
	
	//! Registers the variables of the CSP into the CSP translator
	void createCSPVariables(bool use_novelty_constraint);
	
	//! Common indexing routine - places all conditions and terms appearing in formulas into their right place
	void index_formula_elements(const std::vector<const fs::AtomicFormula*>& conditions, const std::vector<const fs::Term*>& terms);
	
	//! By default, we create no novelty constraint
	virtual void create_novelty_constraint() {}
	
	//! By default, we post no novelty constraint whatsoever
	virtual void post_novelty_constraint(GecodeCSP& csp, const RPGIndex& rpg) const {}
};

} } // namespaces
