
#pragma once

#include <lapkt/novelty/features.hxx>
#include <fs_types.hxx>
#include <state.hxx>

namespace fs0 { namespace language { namespace fstrips { class Term; class AtomicFormula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

using FSFeatureValueT = lapkt::novelty::FeatureValueT;

//! A state variable-based feature that simply returs the value of a certain variable in the state
class StateVariableFeature : public lapkt::novelty::NoveltyFeature<State> {
public:
	StateVariableFeature( VariableIdx variable ) : _variable(variable) {}
	~StateVariableFeature() = default;
	StateVariableFeature(const StateVariableFeature&) = default;
	virtual lapkt::novelty::NoveltyFeature<State>* clone() const override { return new StateVariableFeature(*this); }
	FSFeatureValueT evaluate( const State& s ) const override;
	
	std::ostream& print(std::ostream& os) const override;

protected:
	VariableIdx _variable;
};

//! A feature based on a set of conditions (typically the set of preconditions of an action, 
//! or the goal conditions), that evaluates to the number of satisfied conditions in the set for a given state.
class ConditionSetFeature : public lapkt::novelty::NoveltyFeature<State> {
public:
	ConditionSetFeature() {}
	~ConditionSetFeature() = default;
	
	// we can use the default copy constructor, as formula pointers are NOT owned by this class
	ConditionSetFeature(const ConditionSetFeature&) = default;

	virtual lapkt::novelty::NoveltyFeature<State>* clone() const override { return new ConditionSetFeature(*this); }
	
	void addCondition(const fs::AtomicFormula* condition) { _conditions.push_back(condition); }

	FSFeatureValueT evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override;

protected:
	// formula pointers are NOT owned by this class
	std::vector<const fs::AtomicFormula*> _conditions;
};

//! A feature representing the value of any arbitrary language term, e.g. X+Y, or @proc(Y,Z)
class ArbitraryTermFeature : public lapkt::novelty::NoveltyFeature<State> {
public:
	ArbitraryTermFeature(const fs::Term* term);
	~ArbitraryTermFeature();
	ArbitraryTermFeature(const ArbitraryTermFeature&);
	lapkt::novelty::NoveltyFeature<State>* clone() const override { return new ArbitraryTermFeature(*this); }
	FSFeatureValueT evaluate(const State& s) const override;
	
	std::ostream& print(std::ostream& os) const override;

protected:
	const fs::Term* _term;
};



} // namespaces
