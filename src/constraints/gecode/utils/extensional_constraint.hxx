
#pragma once
#include <vector>


namespace fs0 { namespace language { namespace fstrips { class FluentHeadedNestedTerm; } }}
namespace fs = fs0::language::fstrips;

namespace Gecode { class TupleSet; }
namespace fs0 { class State; }

namespace fs0 { namespace gecode {

class SimpleCSP;
class GecodeCSPVariableTranslator;
class GecodeRPGLayer;

class ExtensionalConstraint {
public:
	
	ExtensionalConstraint(const fs::FluentHeadedNestedTerm* atom);
	
	void register_variables(GecodeCSPVariableTranslator& translator);
	
	void register_constraints(GecodeCSPVariableTranslator& translator);
	
	//! Constraint-posting routines
	bool update(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const State& state) const;
	bool update(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const GecodeRPGLayer& layer) const;
	
// 	const fs::FluentHeadedNestedTerm* getAtom() const { return _atom; }
protected:
	//!
	bool update(SimpleCSP& csp, const GecodeCSPVariableTranslator& translator, const Gecode::TupleSet& extension) const;

	//! The atom that originated the extensional constraint
	const fs::FluentHeadedNestedTerm* _atom;
	
	//! The indexes of the CSP variables that model the value of each of the subterms
	std::vector<unsigned> _subterm_variable_indexes;
	
	//! For 0-ary predicates, we will store in '_variable_idx' the index of the only state variable
	//! that the current atom can yield. This is not only for performance, but because
	//! Gecode cannot handle 0-ary extensional constraints (they don't make much sense anyway)
	//! If the predicate is not 0-ary, '_variable_idx' will be -1.
	int _variable_idx;
};


} } // namespaces

