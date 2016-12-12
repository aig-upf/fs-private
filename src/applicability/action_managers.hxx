
#pragma once

#include <fs_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; class AtomicFormula; } }}
namespace fs = fs0::language::fstrips;


namespace fs0 {

class State;
class GroundAction; 
class Atom;
class TupleIndex;

//! A simple manager that only checks applicability of actions in a non-relaxed setting.
class NaiveApplicabilityManager {
public:
	NaiveApplicabilityManager(const fs::Formula* state_constraints);
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const State& state, const GroundAction& action) const;
	
	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	static std::vector<Atom> computeEffects(const State& state, const GroundAction& action);
	static void computeEffects(const State& state, const GroundAction& action, std::vector<Atom>& atoms);
	
	static bool checkFormulaHolds(const fs::Formula* formula, const State& state);
	
	//! Checks that all of the given new atoms do not violate domain bounds
	static bool checkAtomsWithinBounds(const std::vector<Atom>& atoms);
	
protected:
	//! The state constraints
	const fs::Formula* _state_constraints;
};


class BasicApplicabilityAnalyzer {
public:
	BasicApplicabilityAnalyzer(const std::vector<const GroundAction*>& actions, const TupleIndex& tuple_idx) : 
		_actions(actions), _tuple_idx(tuple_idx) {}
	
	virtual ~BasicApplicabilityAnalyzer() = default;
	
	virtual void build();
	
	const std::vector<std::vector<ActionIdx>>& getApplicable() const { return _applicable; }
	
	unsigned total_actions() const { return _total_actions; }
	
	
protected:
	//! The set of all ground actions managed by this object
	const std::vector<const GroundAction*>& _actions;
	
	//! The tuple index of the problem
	const TupleIndex& _tuple_idx;
	
	//! A map from each atom index to the set of actions that might be applicable when
	//! that atom is true in a certain state
	std::vector<std::vector<ActionIdx>> _applicable;
	
	unsigned _total_actions;
};


class GroundApplicableSet; 
//!
//!
// template <typename ApplicabilityAnalyzerT = BasicApplicabilityAnalyzer>
class SmartActionManager {
public:
	using ApplicableSet = GroundApplicableSet;
	
	SmartActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints, const TupleIndex& tuple_idx, const BasicApplicabilityAnalyzer* analyzer);
	virtual ~SmartActionManager() = default;
	SmartActionManager(const SmartActionManager&) = default;
	
	GroundApplicableSet applicable(const State& state) const;
	
	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool applicable(const State& state, const GroundAction& action) const;
	
protected:
	//! The set of all ground actions managed by this object
	const std::vector<const GroundAction*>& _actions;
	
	//! The state constraints relevant to this object
	const std::vector<const fs::AtomicFormula*> _state_constraints;
	
	//! The tuple index of the problem
	const TupleIndex& _tuple_idx;
	
	//! An index mapping (the index of) each action to the set of state variables affected by that action
	std::vector<std::set<VariableIdx>> _vars_affected_by_actions;
	
	//! An index mapping (the index of) each state constraint action to the set of state variables relevant to that constraint
	std::vector<std::set<VariableIdx>> _vars_relevant_to_constraints;

	//! An index mapping (the index of) each ground action to the state constraints that can be potentially affected by it
	std::vector<std::vector<const fs::AtomicFormula*>> _sc_index;
	
	//! An applicability index that maps each (index of) a tuple (i.e. atom) to the sets of (indexes of) all actions
	//! which are _potentially_ applicable when that atom holds in a state
	const std::vector<std::vector<ActionIdx>>& _app_index;
	
	static std::vector<const fs::AtomicFormula*> process_state_constraints(const fs::Formula* state_constraints);
	
	void index_variables(const std::vector<const GroundAction*>& actions, const std::vector<const fs::AtomicFormula*>& constraints);
	
	virtual bool check_constraints(unsigned action_id, const State& state) const;
	
	//! Computes the list of indexes of those actions that are potentially applicable in the given state
	virtual std::vector<ActionIdx> compute_whitelist(const State& state) const;
	
	friend class GroundApplicableSet;
	
	//! A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
	
	unsigned _total_applicable_actions;
};



//! A simple iterator strategy to iterate over the actions applicable in a given state.
class GroundApplicableSet {
protected:
	friend class SmartActionManager;
	GroundApplicableSet(const SmartActionManager& manager, const State& state, const std::vector<ActionIdx>& action_whitelist);
	
	class Iterator {

		const SmartActionManager& _manager;
		
		const State& _state;
		
		const std::vector<ActionIdx>& _whitelist;
		
		unsigned _index;
		
		void advance();

	public:
		Iterator(const State& state, const SmartActionManager& manager, const std::vector<ActionIdx>& action_whitelist, unsigned index);
		const Iterator& operator++();
		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		ActionIdx operator*() const { return _whitelist[_index]; }
		
		bool operator==(const Iterator &other) const { return _index == other._index; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
public:
	Iterator begin() const { return Iterator(_state, _manager, _whitelist, 0); }
	Iterator end() const { return Iterator(_state, _manager, _whitelist, _whitelist.size()); }

protected:
	const SmartActionManager& _manager;
	
	const State& _state;
	
	const std::vector<ActionIdx> _whitelist;
};







} // namespaces

