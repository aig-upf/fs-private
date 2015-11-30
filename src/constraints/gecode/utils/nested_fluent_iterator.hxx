

#pragma once

#include <languages/fstrips/terms.hxx>
#include <problem.hxx>
#include <utils/cartesian_iterator.hxx>
#include <constraints/gecode/utils/term_list_iterator.hxx>
#include <gecode/int.hh>

namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

//! A nested_fluent_iterator allows us to iterate through all the state variables that can be derived from a given nested fluent term.
class nested_fluent_iterator {
protected:
	//! The nested fluent
	fs::NestedTerm::cptr _term;
	
	term_list_iterator _iterator;
	
	//! The current index in the iteration
	unsigned _index;


public:
	nested_fluent_iterator(fs::NestedTerm::cptr term) :
		_term(term),
		_iterator(term->getSubterms()),
		_index(0)
	{}
	
public:
	const nested_fluent_iterator& operator++() {
		++_iterator;
		++_index;
		return *this;
	}
	const nested_fluent_iterator operator++(int) { nested_fluent_iterator tmp(*this); operator++(); return tmp; }
	
	bool ended() const { return _iterator.ended(); }
	
	unsigned getIndex() const { return _index; }
	
	VariableIdx getDerivedStateVariable() const {
		const ProblemInfo& info = Problem::getInfo();
		return info.resolveStateVariable(_term->getSymbolId(), _iterator.arguments());
	}
	
	Gecode::IntArgs getIndexedIntArgsElement() const {
		return _iterator.getIntArgsElement(getIndex());
	}
};

} } // namespaces
