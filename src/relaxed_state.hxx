
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>
#include <fs0_types.hxx>
#include <atoms.hxx>


namespace fs0 {

class State; class Problem;

class RelaxedState {
protected:
	//! A vector associating symbol IDs to all their possible current extensional value in the relaxed state.
	DomainVector _domains;
	
private:
	//! Private assignment and comparison operators
	RelaxedState& operator=(const RelaxedState &rhs);
// 	bool operator==(const RelaxedState &rhs) const { return _domains == rhs._domains; }
// 	bool operator!=(const RelaxedState &rhs) const { return !(this->operator==(rhs));}

public:
	typedef std::shared_ptr<RelaxedState> ptr;
	typedef std::shared_ptr<const RelaxedState> cptr;

	virtual ~RelaxedState();

	//! Copy constructor - performs a deep copy of the domains
	RelaxedState(const RelaxedState& state) {
		_domains.reserve(state._domains.size());
		
		for (const DomainPtr& domain:state._domains) {
			_domains.push_back(std::make_shared<Domain>(*domain));
		}
	}
	
	//! Construct a relaxed state from a non-relaxed state
	RelaxedState(const State& state);
	
	inline bool operator==(const RelaxedState& rhs){
		if (_domains.size() != rhs._domains.size()) return false;
		for (unsigned i = 0; i < _domains.size(); ++i) {
			if (*(_domains.at(i)) != *(rhs._domains.at(i))) return false;
		}
		return true;
	}
	
	void set(const Atom& fact) {
		_domains.at(fact.getVariable())->insert(fact.getValue());
	}
	
	bool contains(const Atom& fact) const {
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
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const RelaxedState&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	//! Small helper for debugging purposes
	bool checkPointerOwnershipIsCorrect() const;
};

} // namespaces

