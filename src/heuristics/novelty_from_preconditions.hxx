
#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <state.hxx>
#include <problem.hxx>

namespace fs0 {

class NoveltyFromPreconditions;

class NoveltyFromPreconditionsAdapter {
public:
	NoveltyFromPreconditionsAdapter( const State& s, const NoveltyFromPreconditions& featureMap );
	~NoveltyFromPreconditionsAdapter();

	void get_valuation( std::vector< aptk::VariableIndex >& varnames, std::vector< aptk::ValueIndex >& values ) const;
	
protected:
	const State& _adapted;
	const NoveltyFromPreconditions& _featureMap;
};

class NoveltyFeature {
public:
	typedef NoveltyFeature* ptr;

	virtual ~NoveltyFeature() {}
	virtual aptk::ValueIndex evaluate( const State& s ) const = 0;
};

class StateVarFeature : public NoveltyFeature {
public:
	StateVarFeature( VariableIdx variable ) : _variable(variable) {}
	~StateVarFeature() {}
	aptk::ValueIndex  evaluate( const State& s ) const;

protected:
	VariableIdx _variable;
};

class ConstraintSetFeature : public NoveltyFeature {
public:
	ConstraintSetFeature() {}
	
	~ConstraintSetFeature() {}
	
	void addCondition(AtomicFormula::cptr condition) {
		_conditions.push_back(condition);
	}

	aptk::ValueIndex  evaluate( const State& s ) const;

protected:
	std::vector<AtomicFormula::cptr> _conditions;
};


class NoveltyFromPreconditions : public aptk::FiniteDomainNoveltyEvaluator< NoveltyFromPreconditionsAdapter > {
public:
	typedef aptk::FiniteDomainNoveltyEvaluator< NoveltyFromPreconditionsAdapter > BaseClass;

	void selectFeatures( const Problem& problem, bool useStateVars, bool useGoal, bool useActions );

	unsigned evaluate( const State& s ) {
		NoveltyFromPreconditionsAdapter adaptee( s, *this );
		return BaseClass::evaluate( adaptee );
	}

	unsigned numFeatures() const { return _features.size(); }
	NoveltyFeature::ptr feature( unsigned i ) const { return _features[i]; }

	virtual ~NoveltyFromPreconditions();

protected:
	//! 
	std::vector<NoveltyFeature::ptr> _features;
};


}
