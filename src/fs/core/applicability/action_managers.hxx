
#pragma once

#include <unordered_set>

#include <fs/core/fs_types.hxx>
#include <fs/core/applicability/base.hxx>

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
	NaiveApplicabilityManager(const std::vector<const fs::Formula*>& state_constraints);

	//! An action is applicable iff its preconditions hold and its application does not violate any state constraint.
	bool isApplicable(const State& state, const GroundAction& action, bool enforce_state_constraints) const;
    //! A process (natural action) is active if its preconditions hold, note that state constraints
    //! need to be validated over the interval [t, t+dt]
    bool isActive( const State& state, const GroundAction& process ) const;

	//! Note that this might return some repeated atom - and even two contradictory atoms... we don't check that here.
	static std::vector<Atom> computeEffects(const State& state, const GroundAction& action);
	static void computeEffects(const State& state, const GroundAction& action, std::vector<Atom>& atoms);

	static bool checkFormulaHolds(const fs::Formula* formula, const State& state);

    bool checkStateConstraints(const State& s) const;

protected:
	//! The state constraints
	const std::vector<const fs::Formula*>& _state_constraints;
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

	NaiveActionManager(const std::vector<const GroundAction*>& actions, const std::vector<const fs::Formula*>& state_constraints);
	~NaiveActionManager() = default;

	//! Return the set of all actions applicable in a given state
	ApplicableSet applicable(const State& state, bool enforce_state_constraints) const override;

	//! Return whether the given action is applicable in the given state
	bool applicable(const State& state, const GroundAction& action, bool enforce_state_constraints) const override;

	const std::vector<const GroundAction*>& getAllActions() const override { return _actions; }

protected:
	//! The set of all ground actions managed by this object
	const std::vector<const GroundAction*>& _actions;

	//! The state constraints relevant to this object
	const std::vector<const fs::Formula*>& _state_constraints;

	//! A list <0,1, ..., num_actions>
	const std::vector<ActionIdx> _all_actions_whitelist;

	//! Optimization - A cache to hold the effects of the last-applied action and avoid memory allocations.
	mutable std::vector<Atom> _effects_cache;


protected:
	//! Check whether any state constraint is violated in the given state, knowing the last-applied action
	virtual bool check_constraints(unsigned applied_action_id, const State& state) const;

	virtual std::vector<ActionIdx> compute_whitelist(const State& state) const { return _all_actions_whitelist; }
};


//! A simple iterator strategy to iterate over the actions applicable in a given state.
class GroundApplicableSet {
protected:
	friend class NaiveActionManager;

	GroundApplicableSet(const ActionManagerI& manager, const State& state, const std::vector<ActionIdx>& action_whitelist, bool enforce_state_constraints):
		_manager(manager), _state(state), _whitelist(action_whitelist), _enforce_state_constraints(enforce_state_constraints)
	{}

	class Iterator {

		const ActionManagerI& _manager;

		const State& _state;

		const std::vector<ActionIdx>& _whitelist;

		unsigned _index;

		bool _enforce_state_constraints;

		void advance() {

			if (_manager.whitelist_guarantees_applicability()) {
				 // All actions in the whitelist guaranteed to be true, no need to check anything else
				return;
			}

			const std::vector<const GroundAction*>& actions = _manager.getAllActions();

			// std::cout << "Checking applicability " << std::endl;
			for (unsigned sz = _whitelist.size();_index < sz; ++_index) {
					unsigned action_idx = _whitelist[_index];

					if (_manager.applicable(_state, *actions[action_idx], _enforce_state_constraints)) { // The action is applicable, break the for loop.
					// std::cout << "Found applicable action: " << *_actions[action_idx] << std::endl;
					return;
				}
			}

		}

	public:
		Iterator(const State& state, const ActionManagerI& manager, const std::vector<ActionIdx>& action_whitelist, unsigned index, bool enforce_state_constraints) :
			_manager(manager),
			_state(state),
			_whitelist(action_whitelist),
			_index(index),
			_enforce_state_constraints(enforce_state_constraints)
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
	Iterator begin() const { return Iterator(_state, _manager, _whitelist, 0, _enforce_state_constraints); }
	Iterator end() const { return Iterator(_state, _manager, _whitelist, _whitelist.size(), _enforce_state_constraints); }

protected:
	const ActionManagerI& _manager;

	const State& _state;

	const std::vector<ActionIdx> _whitelist;

	bool _enforce_state_constraints;
};



} // namespaces
