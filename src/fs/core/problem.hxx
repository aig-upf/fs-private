
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/atom_index.hxx>

namespace fs0 { namespace language { namespace fstrips { class Formula; class Axiom; class Metric;}}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class State;
class StateAtomIndexer;
class FormulaInterpreter;
class ActionData;
class ActionBase;
class PartiallyGroundedAction;
class GroundAction;

class Problem {
public:
	Problem(    State* init,
                StateAtomIndexer* state_indexer,
                const std::vector<const ActionData*>& action_data,
                const std::unordered_map<std::string, const fs::Axiom*>& axioms,
                const fs::Formula* goal,
                const std::unordered_map<std::string, const fs::Axiom*>& state_constraints,
                const fs::Metric* metric,
                AtomIndex&& tuple_index,
                const AllTransitionGraphsT& transitions);
	~Problem();

	Problem(const Problem& other);
	Problem& operator=(const Problem& other) = delete;
	Problem(Problem&& other) = delete;
	bool operator==(const Problem& other) = delete;
	Problem& operator=(Problem&& rhs) = delete;

	//! Get the initial state of the problem
	const State& getInitialState() const { return *_init; }

	const StateAtomIndexer& getStateAtomIndexer() const { return *_state_indexer; }

	//! Get the set of action schemata of the problem
	const std::vector<const ActionData*>& getActionData() const { return _action_data; }

	//! Get the set of ground actions of the problem
	const std::vector<const GroundAction*>& getGroundActions() const { return _ground; }
	void setGroundActions(std::vector<const GroundAction*>&& ground) { _ground = std::move(ground); }
    void addGroundAction( const GroundAction* a ) { _ground.push_back(a); }

	const std::vector<const PartiallyGroundedAction*>& getPartiallyGroundedActions() const { return _partials; }
	void setPartiallyGroundedActions(std::vector<const PartiallyGroundedAction*>&& actions) { _partials = std::move(actions); }

	//! Get the problem's goal formula
	const fs::Formula* getGoalConditions() const;

	//! Get the problem's metric
	const fs::Metric* get_metric() const { return _metric; }

	//! Get the state constraint formula of the problem
	const std::vector<const fs::Formula*>& getStateConstraints() const { return _state_constraints_formulae; }

    const fs::Axiom* getAxiom(const std::string& name) const {
		auto it = _axioms.find(name);
		if (it == _axioms.end()) return nullptr;
		return it->second;
	}

	const FormulaInterpreter& getGoalSatManager() const { return *_goal_sat_manager; }

	//! Set the global singleton problem instance
	static void setInstance(std::unique_ptr<Problem>&& problem) {
		assert(!_instance);
		_instance = std::move(problem);
	}

	static std::unique_ptr<Problem>&& claimOwnership() {
		return std::move(_instance);
	}

	//! Global singleton object accessor
	static const Problem& getInstance() {
		assert(_instance);
		return *_instance;
	}

	const fs::Axiom* getStateConstraint(const std::string& name) const {
		auto it = _state_constraints.find(name);
		if (it == _state_constraints.end()) return nullptr;
		return it->second;
	}

	const AtomIndex& get_tuple_index() const { return _tuple_index; }

	//! Return true if all the symbols of the problem are predicates
	bool is_predicative() const { return _is_predicative; }

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const Problem& o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

	const AllTransitionGraphsT& get_transition_graphs() const { return _transition_graphs; }

protected:
	//! An index of tuples and atoms
	AtomIndex _tuple_index;

	//! The initial state of the problem
	std::unique_ptr<State> _init;

	const std::unique_ptr<StateAtomIndexer> _state_indexer;

	std::vector<const ActionData*> _action_data;

	//! An index mapping symbol names to the axiomatic definition of the symbol, if it exists.
	std::unordered_map<std::string, const fs::Axiom*> _axioms;

	// The set of grounded actions of the problem
	std::vector<const GroundAction*> _ground;

	// The possible set of partially grounded actions of the problem
	std::vector<const PartiallyGroundedAction*> _partials;

    //! An index mapping symbol names to the axiomatic definition of the symbol, if it exists.
	std::unordered_map<std::string, const fs::Axiom*> _state_constraints;

	//! Pointers to the goal and state constraints formulas. This class owns the pointers.
	std::vector< const fs::Formula*> _state_constraints_formulae;
	const fs::Formula* _goal_formula;

    const fs::Metric*   _metric;

	std::unique_ptr<FormulaInterpreter> _goal_sat_manager;

	//! Whether all the symbols of the problem are predicates
	const bool _is_predicative;

	//! The singleton instance
	static std::unique_ptr<Problem> _instance;

	static bool check_is_predicative();

	AllTransitionGraphsT _transition_graphs;
};

} // namespaces
