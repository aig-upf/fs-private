
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>

#include <fs0_types.hxx>
#include <atoms.hxx>
#include <generic_state.hxx>

namespace fs0 {

class GenericState; class Problem;

class RelaxedGenericState
{
protected:
	//! A vector associating symbol IDs to all their possible current extensional value in the relaxed state.
	DomainVector _domains;
	
private:
	//! Private assignment and comparison operators
	RelaxedGenericState& operator=(const RelaxedGenericState &rhs);
// 	bool operator==(const RelaxedGenericState &rhs) const { return _domains == rhs._domains; }
// 	bool operator!=(const RelaxedGenericState &rhs) const { return !(this->operator==(rhs));}

public:
	typedef std::shared_ptr<RelaxedGenericState> ptr;
	typedef std::shared_ptr<const RelaxedGenericState> cptr;

	virtual ~RelaxedGenericState();

	//! Copy constructor - performs a deep copy of the domains
	RelaxedGenericState(const RelaxedGenericState& state) {
		_domains.reserve(state._domains.size());
		
		for (const DomainPtr& domain:state._domains) {
			_domains.push_back(std::make_shared<Domain>(*domain));
		}
	}
	
	//! Construct a relaxed state from a non-relaxed state
	RelaxedGenericState(const GenericState& state) {
		_domains.reserve(state._values.size());
		
		// For each vector index, we construct a new domain containing only the value from the non-relaxed state.
		for (ObjectIdx value:state._values) {
			DomainPtr domain = std::make_shared<Domain>();
			domain->insert(value);
			_domains.push_back(domain);
		}
	}
	
	inline bool operator==(const RelaxedGenericState& rhs){
		if (_domains.size() != rhs._domains.size()) return false;
		for (unsigned i = 0; i < _domains.size(); ++i) {
			if (*(_domains.at(i)) != *(rhs._domains.at(i))) return false;
		}
		return true;
	}
	
	void set(const Fact& fact) {
		_domains.at(fact.getVariable())->insert(fact.getValue());
	}
	
	bool contains(const Fact& fact) const {
		const auto& possibleValues = getValues(fact.getVariable());
		return possibleValues->find(fact.getValue()) != possibleValues->end();
	}
	
	DomainVector& getDomains() { return _domains; }
	
	//! Const and non-const accessors
	const DomainPtr& getValues(const VariableIdx& variable) const { return _domains.at(variable); }
	DomainPtr getValues(const VariableIdx& variable) { return _domains.at(variable); }
	
	//! Returns the total number of distinct atoms in the relaxed state.
	unsigned getNumberOfAtoms() const {
		unsigned total = 0;
		for (const DomainPtr& domain:_domains) {
			total += domain->size();
		}
		return total;
	}
	
	friend std::ostream& operator<<(std::ostream &os, const RelaxedGenericState&  state) { return state.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const;
	std::ostream& printAll(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const Problem& problem) const;

protected:
	//! Small helper for debugging purposes
	bool checkPointerOwnershipIsCorrect() const;
};

} // namespaces

