
#pragma once

#include <utils/binding.hxx>

namespace fs0 {
class ProblemInfo;
}

namespace fs0 { namespace utils {

class cartesian_iterator;

//! A binding_iterator iterates through all possible bindings of a given signature
class binding_iterator {
protected:
	static const ObjectIdxVector NIL;
	
	std::vector<bool> _valid;
	
	cartesian_iterator* _iterator;

	std::vector<const ObjectIdxVector*> generate_values(const Signature& types, const ProblemInfo& info);
	
public:
	//! Constructor
	binding_iterator(const Signature& types, const ProblemInfo& info);
	~binding_iterator();

	//! Default operators
	binding_iterator(const binding_iterator&) = default;
	binding_iterator(binding_iterator&&) = default;
	binding_iterator& operator=(const binding_iterator&) = default;
	binding_iterator& operator=(binding_iterator&&) = default;
	
	unsigned long num_bindings() const;
	
	//! Generates a fresh binding each time it is invoked
	Binding operator*() const;
	
	const binding_iterator& operator++();
	const binding_iterator operator++(int);
	
	bool ended() const;
};


} } // namespaces
