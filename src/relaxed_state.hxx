
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>
#include <fs0_types.hxx>
#include <atom.hxx>


namespace fs0 {

class State; class Problem;

class RelaxedState {
protected:
	//! A vector associating symbol IDs to all their possible current extensional value in the relaxed state.
	DomainVector _domains;

public:
	typedef std::shared_ptr<RelaxedState> ptr;
	typedef std::shared_ptr<const RelaxedState> cptr;

	virtual ~RelaxedState();

	//! The only way to construct a relaxed state is from a non-relaxed state.
	RelaxedState(const State& state);
	
	// No need to use this - but if ever needed, check the git history!
	// https://bitbucket.org/gfrances/fs0/src/28ce4119f27a537d8f7628c6ca0487d03d5ed0b1/src/relaxed_state.hxx?at=gecode_integration
	RelaxedState(const RelaxedState& state)  = delete;
	RelaxedState& operator=(const RelaxedState& rhs) = delete;
	bool operator==(const RelaxedState& rhs) = delete;
	
	void set(const Atom& atom) {
		set(atom.getVariable(), atom.getValue());
	}
	
	void set(VariableIdx variable, ObjectIdx value) {
		_domains.at(variable)->insert(value);
	}
	
	bool contains(const Atom& fact) const {
		const auto& possibleValues = getValues(fact.getVariable());
		return possibleValues->find(fact.getValue()) != possibleValues->end();
	}
	
	const DomainVector& getDomains() const { return _domains; }
	
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
	
	//!
	unsigned width() const { return _domains.size(); }
	
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const RelaxedState&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;

protected:
	//! Small helper for debugging purposes
	bool checkPointerOwnershipIsCorrect() const;
};

} // namespaces

