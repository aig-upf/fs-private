
#pragma once

#include <lapkt/novelty/features.hxx>
#include <fs_types.hxx>
#include <state.hxx>
#include <memory>

namespace fs0 { namespace language { namespace fstrips { class Term; class AtomicFormula; class Formula; }}}
namespace fs = fs0::language::fstrips;

namespace fs0 {

using FSFeatureValueT = lapkt::novelty::FeatureValueT;

class Feature : public lapkt::novelty::NoveltyFeature<State> {
public:
	Feature();
	Feature( const std::vector<VariableIdx>& scope, type_id codomain  );
	virtual ~Feature();
	virtual const std::vector<VariableIdx>&	scope() const { return _scope; };
	virtual type_id codomain() const { return _codomain; };
protected:
	std::vector<VariableIdx>		_scope;
	type_id							_codomain;
};

//! A state variable-based feature that simply returs the value of a certain variable in the state
class StateVariableFeature : public Feature {
public:
	StateVariableFeature( VariableIdx variable );
	~StateVariableFeature() = default;
	StateVariableFeature(const StateVariableFeature&) = default;
	virtual lapkt::novelty::NoveltyFeature<State>* clone() const override { return new StateVariableFeature(*this); }
	FSFeatureValueT evaluate( const State& s ) const override;

	std::ostream& print(std::ostream& os) const override;

protected:
	VariableIdx 				_variable;
};

//! A feature based on a set of conditions (typically the set of preconditions of an action,
//! or the goal conditions), that evaluates to the number of satisfied conditions in the set for a given state.
class ConditionSetFeature : public Feature {
public:
	ConditionSetFeature();
	~ConditionSetFeature();

	// we can use the default copy constructor, as formula pointers are NOT owned by this class
	ConditionSetFeature(const ConditionSetFeature&) = default;

	virtual lapkt::novelty::NoveltyFeature<State>* clone() const override { return new ConditionSetFeature(*this); }

	void addCondition(const fs::Formula* condition);

	FSFeatureValueT evaluate(const State& s) const override;

	std::ostream& print(std::ostream& os) const override;
protected:
	// formula pointers are NOT owned by this class
	std::vector<const fs::Formula*> _conditions;

};

//! A feature representing the value of any arbitrary language term, e.g. X+Y, or @proc(Y,Z)
class ArbitraryTermFeature : public Feature {
public:
	ArbitraryTermFeature(const fs::Term* term) : Feature(), _term(term) {}
	~ArbitraryTermFeature();
	ArbitraryTermFeature(const ArbitraryTermFeature&);
	lapkt::novelty::NoveltyFeature<State>* clone() const override { return new ArbitraryTermFeature(*this); }
	FSFeatureValueT evaluate(const State& s) const override;

	std::ostream& print(std::ostream& os) const override;

protected:
	const fs::Term* _term;
};

class ArbitraryFormulaFeature : public Feature {
public:
	ArbitraryFormulaFeature(const fs::Formula* formula) : Feature(), _formula(formula) {}
	~ArbitraryFormulaFeature();
	ArbitraryFormulaFeature(const ArbitraryFormulaFeature&);
	lapkt::novelty::NoveltyFeature<State>* clone() const override { return new ArbitraryFormulaFeature(*this); }
	FSFeatureValueT evaluate(const State& s) const override;

	std::ostream& print(std::ostream& os) const override;

protected:
	const fs::Formula* _formula;
};

// MRJ: Why this convoluted design?
// -> We don't want to change the LAPKT class NoveltyFeature<T>
// -> We want to make easy to re-integrate this code into the FS "main" development
//    branch.
//
// Notes:
// -> The 2nd and 3rd template parameters are dependant, we could do away with FeatureDefinition
//    if FeatureT provided a trait "DefinitionT" for instance. We haven't proceeded in this fashion
//    to make the future merge easier.
//
template <typename FeatureDefinition, typename FeatureT, typename Manager >
class ManagedFeature : public FeatureT {
public:
    typedef std::shared_ptr<Manager>    ManagerPtr;

    ManagedFeature(const FeatureDefinition* def, ManagerPtr mgr )
        : FeatureT( def ), _manager(mgr) {}

protected:

    ManagerPtr  _manager;
};

/*
 TODO - WORK-IN-PROGRESS
//! A feature representing any arbitrary procedure that receives a state and returns a feature value
// template <typename FeatureValueT> // TODO GENERALIZE TO BOOL/INT FEATURES, CURRENTLY ONLY INTS ARE SUPPORTED
class LambdaFeature : public lapkt::novelty::NoveltyFeature<State> {
public:
	using EvaluatorT = std::function<int(const State&)>; // TODO Generalize this to return bools too

	LambdaFeature(const EvaluatorT& evaluator) : _evaluator(evaluator) {}
	~LambdaFeature() = default;
// 	LambdaFeature(const LambdaFeature&) = default;

	LambdaFeature* clone() const override { return new LambdaFeature(*this); }
	FSFeatureValueT evaluate(const State& s) const override { // TODO Generalize this to return bools too
		return _evaluator(s);
	}

	std::ostream& print(std::ostream& os) const override {
		return os << "Anonymous Lambda Feature";
	}

protected:
	EvaluatorT _evaluator;
};
*/



} // namespaces
