/**
 * A very basic problem with one action (no preconditions), one unary predicate and 
 * two objects. The action is actually non-parametrized and makes true the 
 * predicate on the two objects.
 */

#include "generator.hxx"

using namespace aptk::core;

namespace aptk { namespace test { namespace problems { namespace simple1 {


OnlyAction::OnlyAction(const SymbolTable& symbols) :
		ParametrizedAction("act", {}),
		pred_ext_idx(symbols.getSymbolId("pred")),
		p_obj_idx(symbols.get_object_id("p")),
		q_obj_idx(symbols.get_object_id("q"))
	{}


// PRECONDITION: TRUE
unsigned OnlyAction::getNumApplicabilityProcedures() const { return 0; }
bool OnlyAction::isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
	assert(point.size()==1);
	return point[0] == true;
}

    
void OnlyAction::getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature, const ParameterBinding& binding) const {
	assert(signature.size() == 0);
}

// EFF: pred(p) AND pred(q)
void OnlyAction::computeChangeset(const CoreState& state, const ParameterBinding& binding, Changeset& changeset) const  {
	changeset.add(Effect(pred_ext_idx, {p_obj_idx}, true));
	changeset.add(Effect(pred_ext_idx, {q_obj_idx}, true));
}


ConcreteGoalEvaluator::ConcreteGoalEvaluator(const SymbolTable& symbols) :
	pred_ext_idx(symbols.getSymbolId("pred")),
	p_obj_idx(symbols.get_object_id("p")),
	q_obj_idx(symbols.get_object_id("q"))
{}

// GOAL: pred(p) and pred(q)
unsigned ConcreteGoalEvaluator::getNumApplicabilityProcedures() const { return 2; }


bool ConcreteGoalEvaluator::isApplicabilityProcedureSatisfied(unsigned procedureIdx, const ProcedurePoint& point) const {
	assert(point.size()==1);
	return point[0] == true;
}

void ConcreteGoalEvaluator::getApplicabilityProcedureRelevantVariables(unsigned procedureIdx, ProcedureSignature& signature) const {
	assert(signature.size() == 0);

	switch(procedureIdx) {
		case 0:
		signature.push_back(StateVariable(pred_ext_idx, {p_obj_idx}));
		break;
		case 1:
		signature.push_back(StateVariable(pred_ext_idx, {q_obj_idx}));
		break;
	}
}


// Return a state where pred is false for all objects.
CoreState generateInitialState(const SymbolTable& symbols) {
    auto pred_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	return CoreState({{symbols.getSymbolId("pred"), pred_ext}});
}

// Return a state where pred is true only for one object
CoreState generateState1(const SymbolTable& symbols) {
    auto pred_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	pred_ext->set({symbols.get_object_id("p")}, true);
	return CoreState({{symbols.getSymbolId("pred"), pred_ext}});
}

// Return the goal state
CoreState generateGoalState(const SymbolTable& symbols) {
    auto pred_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	pred_ext->set({symbols.get_object_id("p")}, true);
	pred_ext->set({symbols.get_object_id("q")}, true);
	return CoreState({{symbols.getSymbolId("pred"), pred_ext}});
}


void generate(aptk::core::Problem& problem) {
    /* Type declaration */
    TypeIdx object_t = problem.add_type("object", 0);

    /* Predicate and function declaration */
    problem.add_predicate("pred", {object_t});

    /* Object and constant declaration */
	problem.add_object("p", object_t);
	problem.add_object("q", object_t);

	const SymbolTable& symbols = problem.close_symbol_table();

	/* Parametrized actions */
	problem.add_parametrized_action(cActionPtr(new OnlyAction(symbols)));

	/* Bind the parametrized actions */
    problem.setActionBinder(std::unique_ptr<ActionBinder>(new ActionBinder(problem)));
	problem.bindActions();

	/* Define the initial state */
	problem.set_initial_state(generateInitialState(symbols));

	/* Define the goal evaluator */
	problem.setGoalEvaluator(cGoalEvaluatorPtr(new ConcreteGoalEvaluator(symbols)));
}

} } } } // namespaces