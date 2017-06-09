
#pragma once

#include <unordered_set>

#include <fs_types.hxx>
#include "base.hxx"

namespace fs0 { namespace language { namespace fstrips { class Term; class Formula; class AtomicFormula; } }}
namespace fs = fs0::language::fstrips;
namespace fs0 {

class State;
class GroundAction;
class Atom;
class AtomIndex;


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
	BasicApplicabilityAnalyzer(const std::vector<const GroundAction*>& actions, const AtomIndex& tuple_idx) :
		_actions(actions), _tuple_idx(tuple_idx) {}

	virtual ~BasicApplicabilityAnalyzer() = default;

	virtual void build(bool build_applicable_index = true);

	const std::vector<std::vector<ActionIdx>>& getApplicable() const { return _applicable; }
	
	const std::vector<std::unordered_set<AtomIdx>>& getRevApplicable() const { return _rev_applicable; }
	
	const std::vector<unsigned>& getVariableRelevance() const { return _variable_relevance; }

	unsigned total_actions() const { return _total_actions; }


protected:
	//! The set of all ground actions managed by this object
	const std::vector<const GroundAction*>& _actions;

	//! The tuple index of the problem
	const AtomIndex& _tuple_idx;

	//! A map from each atom index to the set of actions that might be applicable when
	//! that atom is true in a certain state
	std::vector<std::vector<ActionIdx>> _applicable;
	
	//! A map from each action index to the set of atoms that appear on its precondition
	std::vector<std::unordered_set<AtomIdx>> _rev_applicable;
	
	//! '_variable_relevance[i]' is the number of times that state variable 'i' appears on a (distinct) action precondition
	std::vector<unsigned> _variable_relevance;

	unsigned _total_actions;
};


//! A small helper
object_id _extract_constant_val(const fs::Term* lhs, const fs::Term* rhs);

//!
class NaiveActionManager : public ActionManagerI  {
public:
	using Base = ActionManagerI;
	using ApplicableSet = typename Base::ApplicableSet;

	NaiveActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints);
	~NaiveActionManager() = default;

	//! Return the set of all actions applicable in a given state
	ApplicableSet applicable(const State& state) const override;

	//! Return whether the given action is applicable in the given state
	bool applicable(const State& state, const GroundAction& action) const override;
	
	const std::vector<const GroundAction*>& getAllActions() const override { return _actions; }

protected:
	//! The set of all ground actions managed by this object
	const std::vector<const GroundAction*>& _actions;	
	
	//! The state constraints relevant to this object
	const std::vector<const fs::Formula*> _state_constraints;
	
	//! A list <0,1, ..., num_actions>
	const std::vector<ActionIdx> _all_actions_whitelist;
	
	//! Optimization - A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;
	
	
protected:
	//! Check whether any state constraint is violated in the given state, knowing the last-applied action
	virtual bool check_constraints(unsigned applied_action_id, const State& state) const;
	
	virtual std::vector<ActionIdx> compute_whitelist(const State& state) const { return _all_actions_whitelist; }
};

//! 
class SmartActionManager : public NaiveActionManager {
public:
	using Base = NaiveActionManager;
	using ApplicableSet = typename Base::ApplicableSet;

	SmartActionManager(const std::vector<const GroundAction*>& actions, const fs::Formula* state_constraints, const AtomIndex& tuple_idx, const BasicApplicabilityAnalyzer& analyzer);
	~SmartActionManager() = default;
	SmartActionManager(const SmartActionManager&) = default;

protected:

	//! The tuple index of the problem
	const AtomIndex& _tuple_idx;

	//! An index mapping (the index of) each action to the set of state variables affected by that action
	std::vector<std::set<VariableIdx>> _vars_affected_by_actions;

	//! An index mapping (the index of) each state constraint action to the set of state variables relevant to that constraint
	std::vector<std::set<VariableIdx>> _vars_relevant_to_constraints;

	//! An index mapping (the index of) each ground action to the state constraints that can be potentially affected by it
	std::vector<std::vector<const fs::AtomicFormula*>> _sc_index;

	//! An applicability index that maps each (index of) a tuple (i.e. atom) to the sets of (indexes of) all actions
	//! which are _potentially_ applicable when that atom holds in a state
	const std::vector<std::vector<ActionIdx>>& _app_index;

	void index_variables(const std::vector<const GroundAction*>& actions, const std::vector<const fs::AtomicFormula*>& constraints);

	bool check_constraints(unsigned action_id, const State& state) const override;

	//! Computes the list of indexes of those actions that are potentially applicable in the given state
	std::vector<ActionIdx> compute_whitelist(const State& state) const override;

	unsigned _total_applicable_actions;
};



//! A simple iterator strategy to iterate over the actions applicable in a given state.
class GroundApplicableSet {
protected:
	friend class NaiveActionManager;
	
	GroundApplicableSet(const ActionManagerI& manager, const State& state, const std::vector<ActionIdx>& action_whitelist):
		_manager(manager), _state(state), _whitelist(action_whitelist)
	{}

	class Iterator {

		const ActionManagerI& _manager;

		const State& _state;

		const std::vector<ActionIdx>& _whitelist;

		unsigned _index;

		void advance();

	public:
		Iterator(const State& state, const ActionManagerI& manager, const std::vector<ActionIdx>& action_whitelist, unsigned index) :
			_manager(manager),
			_state(state),
			_whitelist(action_whitelist),
			_index(index)
		{
		 	// std::cout << "Whitelist size: " << _whitelist.size() << " vs. num actions: " << _manager.getAllActions().size() << std::endl;
			advance();
		}

		const Iterator& operator++() {
			++_index;
			advance();
			return *this;
		}

		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		ActionIdx operator*() const { return _whitelist[_index]; }

		bool operator==(const Iterator &other) const { return _index == other._index; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};

public:
	Iterator begin() const { return Iterator(_state, _manager, _whitelist, 0); }
	Iterator end() const { return Iterator(_state, _manager, _whitelist, _whitelist.size()); }

protected:
	const ActionManagerI& _manager;

	const State& _state;

	const std::vector<ActionIdx> _whitelist;
};



} // namespaces
