
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>

#include <core_types.hxx>
#include <fact.hxx>
#include <generic_state.hxx>

namespace aptk { namespace core {

class GenericState; class Problem; class Changeset;

class RelaxedGenericState
{
protected:
	//! A vector associating symbol IDs to all their possible current extensional value in the relaxed state.
	DomainVector _domains;
	
private:
	//! Private Assignment operator
	// TODO - This is probably not exception-safe
	RelaxedGenericState& operator=(const RelaxedGenericState &rhs) {
		if (this == &rhs) return *this;
		_domains = rhs._domains;
		return *this;
	}
	
	
	bool operator==(const RelaxedGenericState &rhs) const { return _domains == rhs._domains; }
	bool operator!=(const RelaxedGenericState &rhs) const { return !(this->operator==(rhs));}	

public:
	typedef std::shared_ptr<RelaxedGenericState> ptr;
	typedef std::shared_ptr<const RelaxedGenericState> cptr;

	virtual ~RelaxedGenericState() {}

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
	
	void set(const Fact& fact) {
		_domains.at(fact._variable)->insert(fact._value);
	}
	
	bool contains(const Fact& fact) const {
		const auto& possibleValues = getValues(fact._variable);
		return possibleValues->find(fact._value) != possibleValues->end();
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
	
	//! Applies the given changeset into the current state.
	void accumulate(const Changeset& changeset);
	
public:
	friend std::ostream& operator<<(std::ostream &os, const RelaxedGenericState&  state) { return state.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const;
	std::ostream& printAll(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const Problem& problem) const;
};

} } // namespaces

