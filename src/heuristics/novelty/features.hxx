
#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <fs_types.hxx>

namespace fs0 { namespace language { namespace fstrips { class Term; class AtomicFormula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

class State;

//! Base interface for any novelty feature
class NoveltyFeature {
public:
	virtual ~NoveltyFeature() = default;
	virtual NoveltyFeature* clone() const = 0;
	virtual aptk::ValueIndex evaluate( const State& s ) const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const NoveltyFeature& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const = 0;
};

//! A state variable-based feature that simply returs the value of a certain variable in the state
class StateVariableFeature : public NoveltyFeature {
public:
	StateVariableFeature( VariableIdx variable ) : _variable(variable) {}
	~StateVariableFeature() = default;
	StateVariableFeature(const StateVariableFeature&) = default;
	virtual NoveltyFeature* clone() const override { return new StateVariableFeature(*this); }
	aptk::ValueIndex  evaluate( const State& s ) const override;
	
	std::ostream& print(std::ostream& os) const override;

protected:
	VariableIdx _variable;
};

//! A feature based on a set of conditions (typically the set of preconditions of an action, 
//! or the goal conditions), that evaluates to the number of satisfied conditions in the set for a given state.
class ConditionSetFeature : public NoveltyFeature {
public:
	ConditionSetFeature() {}
	~ConditionSetFeature() = default;
	
	// we can use the default copy constructor, as formula pointers are NOT owned by this class
	ConditionSetFeature(const ConditionSetFeature&) = default;

	virtual NoveltyFeature* clone() const override { return new ConditionSetFeature(*this); }
	
	void addCondition(const fs::AtomicFormula* condition) { _conditions.push_back(condition); }

	aptk::ValueIndex evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override;

protected:
	// formula pointers are NOT owned by this class
	std::vector<const fs::AtomicFormula*> _conditions;
};

//! A feature representing the value of any arbitrary language term, e.g. X+Y, or @proc(Y,Z)
class ArbitraryTermFeature : public NoveltyFeature {
public:
	ArbitraryTermFeature(const fs::Term* term);
	~ArbitraryTermFeature();
	ArbitraryTermFeature(const ArbitraryTermFeature&);
	NoveltyFeature* clone() const override { return new ArbitraryTermFeature(*this); }
	aptk::ValueIndex evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override;

protected:
	const fs::Term* _term;
};



} // namespaces
