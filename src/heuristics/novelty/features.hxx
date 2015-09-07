#pragma once
#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>

namespace fs0 {

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


}
