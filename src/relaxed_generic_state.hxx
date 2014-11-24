
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>

#include <core_types.hxx>
#include <fact.hxx>

namespace aptk { namespace core {

class GenericState; class Problem; class Changeset;

// Important that this be a set, not an unordered_set, since we want to hash states 
// using boost::hash_range, which resorts to boost::hash_value called on the ObjectSets,
// which in turn makes sense only when applied to ordered containers.
typedef std::set<ObjectIdx> ObjectSet;

class RelaxedGenericState
{
protected:
	//! A vector associating symbol IDs to all their possible current extensional value in the relaxed state.
	std::vector<ObjectSet> _values;

public:
	typedef std::shared_ptr<RelaxedGenericState> ptr;
	typedef std::shared_ptr<const RelaxedGenericState> cptr;

	virtual ~RelaxedGenericState() {}
	
	//! Copy constructor
	RelaxedGenericState(const RelaxedGenericState& state) :
		_values(state._values)
	{}
	
	//! Constructor from a non-relaxed state
	RelaxedGenericState(const GenericState& state) :
		_values(state._values.size()) {
		for (unsigned i = 0; i < _values.size(); ++i) {
			_values[i].insert(state._values[i]);
		}
	}
	
	//! A constructor that receives a changeset and construct a state that is equal to the received
	//! state plus the changes in the changeset.
	RelaxedGenericState(const RelaxedGenericState& state, const Changeset& changeset) :
		_values(state._values) {
		applyChangeset(changeset);
	}
	
	//! Assignment operator
	// TODO - This is probably not exception-safe
	RelaxedGenericState& operator=(const RelaxedGenericState &rhs) {
		if (this == &rhs) return *this;
		_values = rhs._values;
		return *this;
	}
	
	bool operator==(const RelaxedGenericState &rhs) const { return _values == rhs._values; }
	bool operator!=(const RelaxedGenericState &rhs) const { return !(this->operator==(rhs));}
	
	void set(const Fact& fact) {
		_values.at(fact._variable).insert(fact._value);
	}
	
	bool contains(const Fact& fact) const {
		const auto& possibleValues = getValues(fact._variable);
		return possibleValues.find(fact._value) != possibleValues.end();		
	}
	
	const ObjectSet& getValues(const VariableIdx& variable) const {
		return _values.at(variable);
	}
	
protected:
	//! Applies the given changeset into the current state.
	void applyChangeset(const Changeset& changeset);
	
	
public:
	friend std::ostream& operator<<(std::ostream &os, const RelaxedGenericState&  state) { return state.print(os); }
	
	//! Prints a representation of the state to the given stream.
	std::ostream& print(std::ostream& os) const;
	std::ostream& printAll(std::ostream& os) const;
	std::ostream& print(std::ostream& os, const Problem& problem) const;
	
	//! Relax a given state.
	static RelaxedGenericState::ptr relax(const GenericState& s) {
		return std::make_shared<RelaxedGenericState>(s);
	}
};

} } // namespaces

