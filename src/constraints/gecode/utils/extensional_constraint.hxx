
#pragma once

#include <vector>
#include <ostream> 

namespace Gecode { class TupleSet; }
namespace fs0 { class State; class TupleIndex; }
namespace fs0 { namespace language { namespace fstrips { class FluentHeadedNestedTerm; } } }
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class GecodeCSP;
class CSPTranslator;
class RPGIndex;

class ExtensionalConstraint {
public:
	
	ExtensionalConstraint(const fs::FluentHeadedNestedTerm* term, const TupleIndex& tuple_index, bool predicate);
	
	void register_variables(CSPTranslator& translator) {} // TODO - REMOVE IF NOT NEEDED
	
	void register_constraints(CSPTranslator& translator);
	
	const fs::FluentHeadedNestedTerm* get_term() const {return _term; }
	
	//! Constraint-posting routines
	bool update(GecodeCSP& csp, const CSPTranslator& translator, const State& state) const;
	bool update(GecodeCSP& csp, const CSPTranslator& translator, const RPGIndex& layer) const;
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const ExtensionalConstraint&  o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;
	
protected:
	//!
	bool update(GecodeCSP& csp, const CSPTranslator& translator, const Gecode::TupleSet& extension) const;
	
	//! This is a hacky, temporary way of knowing if the current extensional constraint models a predicate or not, in which
	//! case we assume it models a nested fluent.
	bool _predicate;
	unsigned _term_variable_index;

	//! The atom that originated the extensional constraint
	const fs::FluentHeadedNestedTerm* _term;
	
	//! The indexes of the CSP variables that model the value of each of the subterms
	std::vector<unsigned> _subterm_variable_indexes;
	
	//! For 0-ary predicates, we will store in '_variable_idx' the index of the only state variable
	//! that the current atom can yield. This is not only for performance, but because
	//! Gecode cannot handle 0-ary extensional constraints (they don't make much sense anyway)
	//! If the predicate is not 0-ary, '_variable_idx' will be -1.
	int _variable_idx;
	
	//!
	const TupleIndex& _tuple_index;
	
	Gecode::TupleSet compute_extension(unsigned symbol_id, const State& state) const;
};


} } // namespaces

