
#pragma once

#include <fs0_types.hxx>
#include <gecode/int.hh>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/translators/action_translator.hxx>
#include <constraints/gecode/translators/formula_translator.hxx>
#include <languages/fstrips/language.hxx>


namespace fs = fs0::language::fstrips;

namespace fs0 {
	class GroundAction; class RPGData; class RelaxedState;
}


namespace fs0 { namespace gecode {

//! The base interface class for all gecode CSP handlers
class GecodeCSPHandler {
public:
	typedef GecodeCSPHandler* ptr;
	typedef const GecodeCSPHandler* cptr;
	
	GecodeCSPHandler() : _base_csp() {}
	virtual ~GecodeCSPHandler() {}
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const GecodeCSPHandler& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const {
		return translator.print(os, _base_csp);
	}
	
protected:
	
	//! The base Gecode CSP
	SimpleCSP _base_csp;
	
	//! A translator to map planning variables with gecode variables
	GecodeCSPTranslator translator;

};


//! A CSP modeling and solving a logical formula on a certain RPG layer
class GecodeFormulaCSPHandler : public GecodeCSPHandler {
public:
	typedef GecodeFormulaCSPHandler* ptr;
	typedef const GecodeFormulaCSPHandler* cptr;
	
	GecodeFormulaCSPHandler(const std::vector<fs::AtomicFormula::cptr>& conditions);
	~GecodeFormulaCSPHandler() {}
	
	//! Create a new action CSP constraint by the given RPG layer domains
	//! Ownership of the generated pointer belongs to the caller
	SimpleCSP::ptr instantiate_csp(const RelaxedState& layer) const;

	//! Returns true iff the goal CSP is solvable. In that case, extracts the goal supports from the first solution
	bool compute_support(SimpleCSP* csp, Atom::vctr& support, const State& seed) const;
	
	//! Recovers an approximate support for the goal
	void recoverApproximateSupport(SimpleCSP* csp, Atom::vctr& support, const State& seed) const;
	
protected:
	
	//! All the planning variables relevant to the formula CSP.
	VariableIdxVector relevantVariables;
	
	GecodeFormulaTranslator formulaTranslator;
	
	//! Creates the SimpleCSP that corresponds to a certain amount of relevant variables
	void createCSPVariables();
	
	//! Adds constraints to the csp being managed
	void registerConstraints();
	
	//! Helper
	static VariableIdxVector extractRelevantVariables(const std::vector<fs::AtomicFormula::cptr>& conditions);
};

//! A CSP modeling and solving the effect of an action on a certain RPG layer
class GecodeActionCSPHandler : public GecodeCSPHandler {
public:
	typedef GecodeActionCSPHandler* ptr;
	typedef const GecodeActionCSPHandler* cptr;
	
	//!
	GecodeActionCSPHandler(const GroundAction& action);
	~GecodeActionCSPHandler() {}
	
	//! Create a new action CSP constraint by the given RPG layer domains
	//! Ownership of the generated pointer belongs to the caller
	SimpleCSP::ptr instantiate_csp(const RelaxedState& layer) const;

	const GroundAction& getAction() const { return _action; }
	
	void compute_support(SimpleCSP* csp, unsigned actionIdx, RPGData& rpg) const;
	
	
protected:
	const GroundAction& _action;
	
	GecodeActionTranslator actionTranslator;
	
	//! Creates the SimpleCSP that corresponds to a given action.
	void createCSPVariables();

	//! Adds constraints to the csp being managed
	void registerConstraints();
	
	//! Prevents the affected variables to take values already achieved in the previous layer
	// void addNoveltyConstraints(const VariableIdx variable, const RelaxedState& layer, SimpleCSP& csp);
};

//! A CSP modeling and solving the progression between two RPG layers
class GecodeLayerCSPHandler : public GecodeCSPHandler { // TODO
public:
	typedef GecodeLayerCSPHandler* ptr;
	typedef const GecodeLayerCSPHandler* cptr;
	
protected:

};

} } // namespaces
