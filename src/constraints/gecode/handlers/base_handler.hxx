
#pragma once

#include <unordered_set>

#include <fs0_types.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/utils/variable_counter.hxx>
#include <constraints/gecode/utils/nested_fluent_element_translator.hxx>
#include <constraints/gecode/utils/extensional_constraint.hxx>
#include <utils/config.hxx>


namespace fs0 { namespace language { namespace fstrips { class Term; class AtomicFormula; class FluentHeadedNestedTerm; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { class GroundAction; class RPGData; }

namespace fs0 { namespace gecode {

class GecodeRPGLayer;
class GecodeCSPVariableTranslator;
class NoveltyConstraint;

//! The base interface class for all gecode CSP handlers
class BaseCSPHandler {
public:
	typedef BaseCSPHandler* ptr;
	typedef const BaseCSPHandler* cptr;

	BaseCSPHandler(bool approximate, bool dont_care) : _base_csp(), _approximate(approximate), _translator(_base_csp), _novelty(nullptr), _counter(dont_care) {}
	virtual ~BaseCSPHandler();
	
	void init(const RPGData* bookkeeping);

	//! Create a new action CSP constraint by the given RPG layer domains
	//! Ownership of the generated pointer belongs to the caller
	SimpleCSP::ptr instantiate_csp(const GecodeRPGLayer& layer) const;
	SimpleCSP::ptr instantiate_csp(const State& state) const;
	
	const GecodeCSPVariableTranslator& getTranslator() const { return _translator; }

	static void registerTermVariables(const fs::Term* term, CSPVariableType type, GecodeCSPVariableTranslator& translator);
	static void registerTermConstraints(const fs::Term* term, CSPVariableType type, GecodeCSPVariableTranslator& translator);
	static void registerFormulaVariables(const fs::AtomicFormula* condition, GecodeCSPVariableTranslator& translator);
	static void registerFormulaConstraints(const fs::AtomicFormula* condition, GecodeCSPVariableTranslator& translator);

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const BaseCSPHandler& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const { return print(os, _base_csp); }
	std::ostream& print(std::ostream& os, const SimpleCSP& csp) const;
	
protected:
	//! The base Gecode CSP
	SimpleCSP _base_csp;
	
	//! Whether to solve the CSPs completely or approximately
	bool _approximate;

	//! A translator to map planning variables with gecode variables
	GecodeCSPVariableTranslator _translator;
	
	NoveltyConstraint* _novelty;
	
	VariableCounter _counter;
	
	// All (distinct) FSTRIPS terms that participate in the CSP
	std::unordered_set<const fs::Term*> _all_terms;
	
	// All (distinct) FSTRIPS atomic formulas that participate in the CSP
	std::unordered_set<const fs::AtomicFormula*> _all_formulas;
	
	//! All (distinct) FSTRIPS atoms that participate in the CSP
	std::unordered_set<const fs::FluentHeadedNestedTerm*> _all_atoms;
	
	//! The Ids of the state variables that are relevant to some formula via a (predicative) atom.
	std::set<Atom> _atom_state_variables;
	
	//! The set of nested fluent translators, one for each nested fluent in the set of terms modeled by this CSP
	std::vector<NestedFluentElementTranslator> _nested_fluent_translators;
	std::vector<ExtensionalConstraint> _extensional_constraints;
	
	//! An index from the actual term to the position of the translator in the vector '_nested_fluent_translators'
	//! Note that we need to use the hash and equality specializations of the parent class Term pointer
	std::unordered_map<const fs::FluentHeadedNestedTerm*, unsigned, std::hash<const fs::Term*>, std::equal_to<const fs::Term*>> _nested_fluent_translators_idx;
	
	//! Return the nested fluent translator that corresponds to the given term
	const NestedFluentElementTranslator& getNestedFluentTranslator(const fs::FluentHeadedNestedTerm* fluent) const;
	
	virtual void create_novelty_constraint() = 0;
	
	//! Index all terms and formulas appearing in the formula / actions which will be relevant to the CSP
	virtual void index() = 0;

	void setup();
	
	//! Count which state variables appear directly and/or derivedly, so that we can decide what type of don't care constraints we can use, etc.
	void count_variables();
	
	void register_csp_variables();

	void register_csp_constraints();
	
	//! Registers the variables of the CSP into the CSP translator
	void createCSPVariables(bool use_novelty_constraint);
	
	//! Common indexing routine - places all conditions and terms appearing in formulas into their right place
	void index_formula_elements(const std::vector<const fs::AtomicFormula*>& conditions, const std::vector<fs::Term::cptr>& terms);

};


//! A CSP modeling and solving the progression between two RPG layers
class GecodeLayerCSPHandler : public BaseCSPHandler { // TODO
public:
	typedef GecodeLayerCSPHandler* ptr;
	typedef const GecodeLayerCSPHandler* cptr;

protected:

};

} } // namespaces
