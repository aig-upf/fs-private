
#pragma once

#include <aptk2/heuristics/novelty/fd_novelty_evaluator.hxx>
#include <fs0_types.hxx>
#include <languages/fstrips/formulae.hxx>

namespace fs = fs0::language::fstrips;

namespace fs0 {
	class State;

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

		void addCondition(fs::AtomicFormula::cptr condition) {
			_conditions.push_back(condition);
		}

		aptk::ValueIndex  evaluate( const State& s ) const;

	protected:
		std::vector<fs::AtomicFormula::cptr> _conditions;
	};


}
