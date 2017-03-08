
#pragma once

#include <core_parametrized_action.hxx>
#include <core_goal_evaluator.hxx>
#include <core_problem.hxx>
#include <boost/concept_check.hpp>
#include <core_state.hxx>

using namespace aptk::core;

namespace aptk { namespace test { namespace problems { namespace simple1 {


class OnlyAction : public ParametrizedAction {

protected:
    const SymbolIdx pred_ext_idx;
    const ObjectIdx p_obj_idx;
	const ObjectIdx q_obj_idx;

public:
    
	OnlyAction(const SymbolTable& symbols);

// 	bool isFeasible(const ParameterBinding& binding) const { return true; }

	// PRECONDITION: TRUE
	virtual unsigned getNumApplicabilityProcedures() const;
	virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const;

    
	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature, const ParameterBinding& binding) const;

	// EFF: pred(p) AND pred(q)
	void computeChangeset(const CoreState& state, const ParameterBinding& binding, Changeset& changeset) const;
};

CoreState generateInitialState(const SymbolTable& symbols);

// Return a state where pred is true only for one object
CoreState generateState1(const SymbolTable& symbols);

// Return the goal state
CoreState generateGoalState(const SymbolTable& symbols);

/*********************************************/
/* The goal evaluator                        */
/*********************************************/

class ConcreteGoalEvaluator : public GoalEvaluator {

protected:
    const SymbolIdx pred_ext_idx;
    const ObjectIdx p_obj_idx;
	const ObjectIdx q_obj_idx;

public:
	ConcreteGoalEvaluator(const SymbolTable& symbols);

	virtual unsigned getNumApplicabilityProcedures() const;

    virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const;

	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature) const;
};

void generate(aptk::core::Problem& problem);

} } } } // namespaces
