

#pragma once

#include <gecode/int.hh>

namespace fs0 { namespace language { namespace fstrips { class Term; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace utils { class cartesian_iterator; }}

namespace fs0 { namespace gecode {

//! A term_list_iterator allows us to iterate through all the state variables that can be derived from a given nested fluent term.
class term_list_iterator {
protected:
	//! The list of terms to be iterated through
	const std::vector<const fs::Term*>& _subterms;
	
	// We store the temporary vectors to delete them afterwards
	std::vector<const std::vector<object_id>*> _temporary_vectors;
	
	//! The iterator of the actual cartesian product of values
	utils::cartesian_iterator* _iterator;
	
	std::vector<const std::vector<object_id>*> compute_possible_values(const std::vector<const fs::Term*>& subterms);

public:
	term_list_iterator(const std::vector<const fs::Term*>& subterms);
	~term_list_iterator();
	
public:
	const term_list_iterator& operator++();
	
	const term_list_iterator operator++(int) { term_list_iterator tmp(*this); operator++(); return tmp; }
	
	bool ended() const;
	
	Gecode::IntArgs getIntArgsElement(int element) const;
	
	Gecode::IntArgs getIntArgsElement() const;
	
	const std::vector<object_id>& arguments() const;

};

} } // namespaces
