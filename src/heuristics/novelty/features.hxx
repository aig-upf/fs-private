
#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <fs_types.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {
	
class State;

//! Base interface for any novelty feature
class NoveltyFeature {
public:
	typedef NoveltyFeature* ptr;

	virtual ~NoveltyFeature() {}
	virtual aptk::ValueIndex evaluate( const State& s ) const = 0;
};

//! A state variable-based feature that simply returs the value of a certain variable in the state
class StateVariableFeature : public NoveltyFeature {
public:
	StateVariableFeature( VariableIdx variable ) : _variable(variable) {}
	~StateVariableFeature() {}
	aptk::ValueIndex  evaluate( const State& s ) const;

protected:
	VariableIdx _variable;
};

//! A feature based on a set of conditions (typically the set of preconditions of an action, 
//! or the goal conditions), that evaluates to the number of satisfied conditions in the set for a given state.
class ConditionSetFeature : public NoveltyFeature {
public:
	ConditionSetFeature() {}
	~ConditionSetFeature() {}

	void addCondition(const fs::AtomicFormula* condition) { _conditions.push_back(condition); }

	aptk::ValueIndex  evaluate( const State& s ) const;

protected:
	std::vector<const fs::AtomicFormula*> _conditions;
};

} // namespaces
