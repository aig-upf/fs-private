
#ifndef __APTK_CORE_TEST_PROBLEMS_BLOCKS_1_GENERATOR__
#define __APTK_CORE_TEST_PROBLEMS_BLOCKS_1_GENERATOR__

#include <core_parametrized_action.hxx>
#include <core_goal_evaluator.hxx>
#include <core_problem.hxx>
#include <boost/concept_check.hpp>
#include <core_state.hxx>
#include <core_types.hxx>
#include <symbol_table.hxx>
#include <core_goal_evaluator.hxx>
#include <core_changeset.hxx>
#include <core_effect.hxx>
#include <exception>

using namespace aptk::core;

namespace aptk { namespace test { namespace problems { namespace blocks1 {


void generate(aptk::core::Problem& problem);

CoreState generateInitialState(const SymbolTable& symbols);

CoreState generateInvertedGoalState(const SymbolTable& symbols);

CoreState generateIncGoalState(const SymbolTable& symbols);

CoreState generateGoalState(const SymbolTable& symbols);
  

/*********************************************/
/* The actions                               */
/*********************************************/

class PickUpAction : public ParametrizedAction {

protected:
    const SymbolIdx on_ext_idx;
	const SymbolIdx ontable_ext_idx;
	const SymbolIdx clear_ext_idx;
	const SymbolIdx handempty_ext_idx;
	const SymbolIdx holding_ext_idx;
    const ObjectIdx d_obj_idx;
	const ObjectIdx b_obj_idx;
	const ObjectIdx a_obj_idx;
	const ObjectIdx c_obj_idx;

public:
    
PickUpAction(const SymbolTable& symbols) :
		ParametrizedAction("pick-up", {symbols.get_type_id("object")}),
		on_ext_idx(symbols.getSymbolId("on")),
		ontable_ext_idx(symbols.getSymbolId("ontable")),
		clear_ext_idx(symbols.getSymbolId("clear")),
		handempty_ext_idx(symbols.getSymbolId("handempty")),
		holding_ext_idx(symbols.getSymbolId("holding")),
		d_obj_idx(symbols.get_object_id("d")),
		b_obj_idx(symbols.get_object_id("b")),
		a_obj_idx(symbols.get_object_id("a")),
		c_obj_idx(symbols.get_object_id("c"))
	{}

	bool isFeasible(const ParameterBinding& binding) const {
		return true;
	}

	virtual unsigned getNumApplicabilityProcedures() const { return 3; }

	
    virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
        assert(point.size()==1);
        
		switch(procedureIdx) {
			case 0:
			return point[0] == true;
			break;
			case 1:
			return point[0] == true;
			break;
			case 2:
			return point[0] == true;
			break;
		}
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}


    
	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature, const ParameterBinding& binding) const {
		assert(signature.size() == 0);

		ObjectIdx x_idx = binding[0];

		switch(procedureIdx) {
			case 0:
			signature.push_back(StateVariable(clear_ext_idx, {x_idx}));
			break;
			case 1:
			signature.push_back(StateVariable(ontable_ext_idx, {x_idx}));
			break;
			case 2:
			signature.push_back(StateVariable(handempty_ext_idx, {}));
			break;
		}
	}

	
	void computeChangeset(const CoreState& state, const ParameterBinding& binding, Changeset& changeset) const  {
		ObjectIdx x_idx = binding[0];
		
		changeset.add(Effect(ontable_ext_idx, {x_idx}, false));
		changeset.add(Effect(clear_ext_idx, {x_idx}, false));
		changeset.add(Effect(handempty_ext_idx, {}, false));
		changeset.add(Effect(holding_ext_idx, {x_idx}, true));
	}
};


class PutDownAction : public ParametrizedAction {

protected:
    const SymbolIdx on_ext_idx;
	const SymbolIdx ontable_ext_idx;
	const SymbolIdx clear_ext_idx;
	const SymbolIdx handempty_ext_idx;
	const SymbolIdx holding_ext_idx;
    const ObjectIdx d_obj_idx;
	const ObjectIdx b_obj_idx;
	const ObjectIdx a_obj_idx;
	const ObjectIdx c_obj_idx;

public:
    
PutDownAction(const SymbolTable& symbols) :
		ParametrizedAction("put-down", {symbols.get_type_id("object")}),
		on_ext_idx(symbols.getSymbolId("on")),
		ontable_ext_idx(symbols.getSymbolId("ontable")),
		clear_ext_idx(symbols.getSymbolId("clear")),
		handempty_ext_idx(symbols.getSymbolId("handempty")),
		holding_ext_idx(symbols.getSymbolId("holding")),
		d_obj_idx(symbols.get_object_id("d")),
		b_obj_idx(symbols.get_object_id("b")),
		a_obj_idx(symbols.get_object_id("a")),
		c_obj_idx(symbols.get_object_id("c"))
	{}

	bool isFeasible(const ParameterBinding& binding) const {
		return true;
	}

	virtual unsigned getNumApplicabilityProcedures() const { return 1; }

	
    virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
        assert(point.size()==1);
        
		switch(procedureIdx) {
			case 0:
			return point[0] == true;
			break;
		}
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}


    
	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature, const ParameterBinding& binding) const {
		assert(signature.size() == 0);

		ObjectIdx x_idx = binding[0];

		switch(procedureIdx) {
			case 0:
			signature.push_back(StateVariable(holding_ext_idx, {x_idx}));
			break;
		}
	}

	
	void computeChangeset(const CoreState& state, const ParameterBinding& binding, Changeset& changeset) const  {
		ObjectIdx x_idx = binding[0];
		
		changeset.add(Effect(holding_ext_idx, {x_idx}, false));
		changeset.add(Effect(clear_ext_idx, {x_idx}, true));
		changeset.add(Effect(handempty_ext_idx, {}, true));
		changeset.add(Effect(ontable_ext_idx, {x_idx}, true));
	}
};


class StackAction : public ParametrizedAction {

protected:
    const SymbolIdx on_ext_idx;
	const SymbolIdx ontable_ext_idx;
	const SymbolIdx clear_ext_idx;
	const SymbolIdx handempty_ext_idx;
	const SymbolIdx holding_ext_idx;
    const ObjectIdx d_obj_idx;
	const ObjectIdx b_obj_idx;
	const ObjectIdx a_obj_idx;
	const ObjectIdx c_obj_idx;

public:
    
StackAction(const SymbolTable& symbols) :
		ParametrizedAction("stack", {symbols.get_type_id("object"),symbols.get_type_id("object")}),
		on_ext_idx(symbols.getSymbolId("on")),
		ontable_ext_idx(symbols.getSymbolId("ontable")),
		clear_ext_idx(symbols.getSymbolId("clear")),
		handempty_ext_idx(symbols.getSymbolId("handempty")),
		holding_ext_idx(symbols.getSymbolId("holding")),
		d_obj_idx(symbols.get_object_id("d")),
		b_obj_idx(symbols.get_object_id("b")),
		a_obj_idx(symbols.get_object_id("a")),
		c_obj_idx(symbols.get_object_id("c"))
	{}

	bool isFeasible(const ParameterBinding& binding) const {
		return true;
	}

	virtual unsigned getNumApplicabilityProcedures() const { return 2; }

	
    virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
        assert(point.size()==1);
        
		switch(procedureIdx) {
			case 0:
			return point[0] == true;
			break;
			case 1:
			return point[0] == true;
			break;
		}
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}


    
	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature, const ParameterBinding& binding) const {
		assert(signature.size() == 0);

		ObjectIdx x_idx = binding[0];
		ObjectIdx y_idx = binding[1];

		switch(procedureIdx) {
			case 0:
			signature.push_back(StateVariable(holding_ext_idx, {x_idx}));
			break;
			case 1:
			signature.push_back(StateVariable(clear_ext_idx, {y_idx}));
			break;
		}
	}

	
	void computeChangeset(const CoreState& state, const ParameterBinding& binding, Changeset& changeset) const  {
		ObjectIdx x_idx = binding[0];
		ObjectIdx y_idx = binding[1];
		
		changeset.add(Effect(holding_ext_idx, {x_idx}, false));
		changeset.add(Effect(clear_ext_idx, {y_idx}, false));
		changeset.add(Effect(clear_ext_idx, {x_idx}, true));
		changeset.add(Effect(handempty_ext_idx, {}, true));
		changeset.add(Effect(on_ext_idx, {x_idx,y_idx}, true));
	}
};


class UnstackAction : public ParametrizedAction {

protected:
    const SymbolIdx on_ext_idx;
	const SymbolIdx ontable_ext_idx;
	const SymbolIdx clear_ext_idx;
	const SymbolIdx handempty_ext_idx;
	const SymbolIdx holding_ext_idx;
    const ObjectIdx d_obj_idx;
	const ObjectIdx b_obj_idx;
	const ObjectIdx a_obj_idx;
	const ObjectIdx c_obj_idx;

public:
    
UnstackAction(const SymbolTable& symbols) :
		ParametrizedAction("unstack", {symbols.get_type_id("object"),symbols.get_type_id("object")}),
		on_ext_idx(symbols.getSymbolId("on")),
		ontable_ext_idx(symbols.getSymbolId("ontable")),
		clear_ext_idx(symbols.getSymbolId("clear")),
		handempty_ext_idx(symbols.getSymbolId("handempty")),
		holding_ext_idx(symbols.getSymbolId("holding")),
		d_obj_idx(symbols.get_object_id("d")),
		b_obj_idx(symbols.get_object_id("b")),
		a_obj_idx(symbols.get_object_id("a")),
		c_obj_idx(symbols.get_object_id("c"))
	{}

	bool isFeasible(const ParameterBinding& binding) const {
		return true;
	}

	virtual unsigned getNumApplicabilityProcedures() const { return 3; }

	
    virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
        assert(point.size()==1);
        
		switch(procedureIdx) {
			case 0:
			return point[0] == true;
			break;
			case 1:
			return point[0] == true;
			break;
			case 2:
			return point[0] == true;
			break;
		}
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}


    
	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature, const ParameterBinding& binding) const {
		assert(signature.size() == 0);

		ObjectIdx x_idx = binding[0];
		ObjectIdx y_idx = binding[1];

		switch(procedureIdx) {
			case 0:
			signature.push_back(StateVariable(on_ext_idx, {x_idx,y_idx}));
			break;
			case 1:
			signature.push_back(StateVariable(clear_ext_idx, {x_idx}));
			break;
			case 2:
			signature.push_back(StateVariable(handempty_ext_idx, {}));
			break;
		}
	}

	
	void computeChangeset(const CoreState& state, const ParameterBinding& binding, Changeset& changeset) const  {
		ObjectIdx x_idx = binding[0];
		ObjectIdx y_idx = binding[1];
		
		changeset.add(Effect(holding_ext_idx, {x_idx}, true));
		changeset.add(Effect(clear_ext_idx, {y_idx}, true));
		changeset.add(Effect(clear_ext_idx, {x_idx}, false));
		changeset.add(Effect(handempty_ext_idx, {}, false));
		changeset.add(Effect(on_ext_idx, {x_idx,y_idx}, false));
	}
};


/*********************************************/
/* The goal evaluator                        */
/*********************************************/

class ConcreteGoalEvaluator : public GoalEvaluator {

protected:
    const SymbolIdx on_ext_idx;
	const SymbolIdx ontable_ext_idx;
	const SymbolIdx clear_ext_idx;
	const SymbolIdx handempty_ext_idx;
	const SymbolIdx holding_ext_idx;
    const ObjectIdx d_obj_idx;
	const ObjectIdx b_obj_idx;
	const ObjectIdx a_obj_idx;
	const ObjectIdx c_obj_idx;

public:
    
	ConcreteGoalEvaluator(const SymbolTable& symbols) :
		on_ext_idx(symbols.getSymbolId("on")),
		ontable_ext_idx(symbols.getSymbolId("ontable")),
		clear_ext_idx(symbols.getSymbolId("clear")),
		handempty_ext_idx(symbols.getSymbolId("handempty")),
		holding_ext_idx(symbols.getSymbolId("holding")),
		d_obj_idx(symbols.get_object_id("d")),
		b_obj_idx(symbols.get_object_id("b")),
		a_obj_idx(symbols.get_object_id("a")),
		c_obj_idx(symbols.get_object_id("c"))
	{}

	virtual unsigned getNumApplicabilityProcedures() const { return 3; }

	
    virtual bool isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
        assert(point.size()==1);
        
		switch(procedureIdx) {
			case 0:
			return point[0] == true;
			break;
			case 1:
			return point[0] == true;
			break;
			case 2:
			return point[0] == true;
			break;
		}
		throw std::runtime_error("Shouldn't reach this point - wrong procedure index");
	}


    
	virtual void getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature) const {
		assert(signature.size() == 0);

		

		switch(procedureIdx) {
			case 0:
			signature.push_back(StateVariable(on_ext_idx, {d_obj_idx,c_obj_idx}));
			break;
			case 1:
			signature.push_back(StateVariable(on_ext_idx, {c_obj_idx,b_obj_idx}));
			break;
			case 2:
			signature.push_back(StateVariable(on_ext_idx, {b_obj_idx,a_obj_idx}));
			break;
		}
	}
};



} } } } // namespaces

#endif