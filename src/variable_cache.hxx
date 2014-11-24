
#ifndef __APTK_CORE_VARIABLE_CACHE_HXX__
#define __APTK_CORE_VARIABLE_CACHE_HXX__

#include <iostream>
#include <cstdlib>
#include <memory>
#include <map>
#include <vector>

#include <fact.hxx>

namespace aptk { namespace core {

class VariableCache
{
protected:
	//!  A vector associating variable IDs to their cached values.
	std::map<unsigned, std::vector<ObjectIdx>> _values;

public:
	typedef std::shared_ptr<VariableCache> ptr;
	typedef std::shared_ptr<const VariableCache> cptr;

	//! Construct an empty cache.
	VariableCache() :
		_values() {};
		
	virtual ~VariableCache() {}
	
	
	void set(unsigned variable, ObjectIdx value) {
		_values.at(variable).push_back(value);
	}
	
	const std::map<unsigned, std::vector<ObjectIdx>>& getMap() const { return _values; }
	std::map<unsigned, std::vector<ObjectIdx>>& getMap() { return _values; }
	
// 	const std::vector<ObjectIdx>& getValues(const unsigned& variable) const {
// 		return _values.find(variable);
// 	}

	void clear() {
		_values.clear();
	}

public:
	friend std::ostream& operator<<(std::ostream &os, const VariableCache&  cache) { return cache.print(os); }
	
	//! Prints a representation of the cache to the given stream.
	std::ostream& print(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const Problem& problem) const;

};

} } // namespaces

#endif

