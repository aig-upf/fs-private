
#pragma once

#include <fs_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; class AtomicFormula; } }}
namespace fs = fs0::language::fstrips;


namespace fs0 {

class State;
class GroundAction; 
class Atom;

//! A simple manager that only checks applicability of actions in a non-relaxed setting.
class NaiveApplicabilityManager {
public:
	NaiveApplicabilityManager(const fs::Formula* state_constraints);
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const State& state, const GroundAction& action) const;
	
	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	static std::vector<Atom> computeEffects(const State& state, const GroundAction& action);
	
	static bool checkFormulaHolds(const fs::Formula* formula, const State& state);
	
	//! Checks that all of the given new atoms do not violate domain bounds
	static bool checkAtomsWithinBounds(const std::vector<Atom>& atoms);
	
protected:
	//! The state constraints
	const fs::Formula* _state_constraints;
};

class GroundApplicableSet; 
//!
//!
class SmartActionManager {
public:
	using ApplicableSet = GroundApplicableSet;
	
	SmartActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints);
	~SmartActionManager() = default;
	
	GroundApplicableSet applicable(const State& state) const;
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool applicable(const State& state, const GroundAction& action) const;
	
protected:
	//!
	const std::vector<const GroundAction*>& _actions;
	
	//! The state constraints
	const std::vector<const fs::AtomicFormula*> _state_constraints;
	
	std::vector<std::set<VariableIdx>> _vars_affected_by_actions;
	
	std::vector<std::set<VariableIdx>> _vars_relevant_to_constraints;

	//! An index mapping (the index of) each ground action to the state constraints that can be potentially affected by it
	std::vector<std::vector<const fs::AtomicFormula*>> _sc_index;
	
	
	static std::vector<const fs::AtomicFormula*> process_state_constraints(const fs::Formula* state_constraints);
	
	void index_variables(const std::vector<const GroundAction*>& actions, const std::vector<const fs::AtomicFormula*>& constraints);
	
	bool check_constraints(unsigned action_id, const State& state) const;
	
	friend class GroundApplicableSet;
};



//! A simple iterator strategy to iterate over the actions applicable in a given state.
class GroundApplicableSet {
public:
	GroundApplicableSet(const SmartActionManager& manager, const State& state);
	
	class Iterator {

		const SmartActionManager& _manager;
		
		const State& _state;
		
		unsigned _index;
		
		void advance();

	public:
		Iterator(const State& state, const SmartActionManager& manager, unsigned index);
		const Iterator& operator++();
		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		ActionIdx operator*() const { return _index; }
		
		bool operator==(const Iterator &other) const { return _index == other._index; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
	Iterator begin() const { return Iterator(_state, _manager, 0); }
	Iterator end() const { return Iterator(_state, _manager, _manager._actions.size()); }

protected:
	const SmartActionManager& _manager;
	
	const State& _state;	
};







} // namespaces

