
#pragma once

#include <fs0_types.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace language { namespace fstrips { class NestedTerm; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class term_list_iterator;

//! A nested_fluent_iterator allows us to iterate through all the state variables that can be derived from a given nested fluent term.
class nested_fluent_iterator {
protected:
	//! The nested fluent
	const fs::NestedTerm* _term;
	
	term_list_iterator* _iterator;
	
	//! The current index in the iteration
	unsigned _index;

public:
	nested_fluent_iterator(const fs::NestedTerm* term);
	~nested_fluent_iterator();
	
	const nested_fluent_iterator& operator++();
	
	const nested_fluent_iterator operator++(int);
	
	bool ended() const;
	
	unsigned getIndex() const;
	
	VariableIdx getDerivedStateVariable() const;
	
	Gecode::IntArgs getIndexedIntArgsElement() const;
};

} } // namespaces
