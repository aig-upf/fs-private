
#pragma once

#include <utils/binding.hxx>
#include <utils/cartesian_iterator.hxx>

namespace fs0 {
class ProblemInfo;
}

namespace fs0 { namespace utils {

//! A binding_iterator
class binding_iterator {
protected:
	static const ObjectIdxVector NIL;
	
	std::vector<bool> _valid;
	
	cartesian_iterator _iterator;

	std::vector<const ObjectIdxVector*> generate_values(const std::vector<TypeIdx>& types, const ProblemInfo& info);
	
public:
	//! Constructor
	binding_iterator(const std::vector<TypeIdx>& types, const ProblemInfo& info);

	//! Default operators
	binding_iterator(const binding_iterator&) = default;
	binding_iterator(binding_iterator&&) = default;
	binding_iterator& operator=(const binding_iterator&) = default;
	binding_iterator& operator=(binding_iterator&&) = default;
	
	unsigned num_bindings() const { return _iterator.size(); }
	
	//! Generates a fresh binding each time it is invoked
	Binding operator*() const { 
		return Binding(*_iterator, _valid);
	}
	
	const binding_iterator& operator++() {
		++_iterator;
		return *this;
	}
	const binding_iterator operator++(int) {binding_iterator tmp(*this); operator++(); return tmp;}
	
	bool ended() const { return _iterator.ended(); }
};


} } // namespaces
