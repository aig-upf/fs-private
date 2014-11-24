/**
 * The blocksworld_4_0 benchmark instance.
 */

#include "generator.hxx"

using namespace aptk::core;

namespace aptk { namespace test { namespace problems { namespace blocks1 {


CoreState generateInitialState(const SymbolTable& symbols) {
    auto on_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto ontable_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto clear_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto handempty_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto holding_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());

    clear_ext->set({symbols.get_object_id("a")}, true);
	clear_ext->set({symbols.get_object_id("d")}, true);
	clear_ext->set({symbols.get_object_id("b")}, true);
	ontable_ext->set({symbols.get_object_id("c")}, true);
	handempty_ext->set({}, true);
	ontable_ext->set({symbols.get_object_id("a")}, true);
	ontable_ext->set({symbols.get_object_id("d")}, true);
	ontable_ext->set({symbols.get_object_id("b")}, true);
	clear_ext->set({symbols.get_object_id("c")}, true);

	return CoreState({{symbols.getSymbolId("on"), on_ext},
		{symbols.getSymbolId("ontable"), ontable_ext},
		{symbols.getSymbolId("clear"), clear_ext},
		{symbols.getSymbolId("handempty"), handempty_ext},
		{symbols.getSymbolId("holding"), holding_ext}});
}

CoreState generateInvertedGoalState(const SymbolTable& symbols) {
    auto on_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto ontable_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto clear_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto handempty_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto holding_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());

    clear_ext->set({symbols.get_object_id("a")}, true);
	
	on_ext->set({symbols.get_object_id("a"), symbols.get_object_id("b")}, true);
	on_ext->set({symbols.get_object_id("b"), symbols.get_object_id("c")}, true);
	on_ext->set({symbols.get_object_id("c"), symbols.get_object_id("d")}, true);
	ontable_ext->set({symbols.get_object_id("d")}, true);
	
	handempty_ext->set({}, true);
	
	return CoreState({{symbols.getSymbolId("on"), on_ext},
		{symbols.getSymbolId("ontable"), ontable_ext},
		{symbols.getSymbolId("clear"), clear_ext},
		{symbols.getSymbolId("handempty"), handempty_ext},
		{symbols.getSymbolId("holding"), holding_ext}});
}



CoreState generateIncGoalState(const SymbolTable& symbols) {
    auto on_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto ontable_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto clear_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto handempty_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto holding_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());

    clear_ext->set({symbols.get_object_id("c")}, true);
	
	on_ext->set({symbols.get_object_id("a"), symbols.get_object_id("d")}, true);
	on_ext->set({symbols.get_object_id("b"), symbols.get_object_id("a")}, true);
	on_ext->set({symbols.get_object_id("c"), symbols.get_object_id("b")}, true);
	ontable_ext->set({symbols.get_object_id("d")}, true);
	
	handempty_ext->set({}, true);
	
	return CoreState({{symbols.getSymbolId("on"), on_ext},
		{symbols.getSymbolId("ontable"), ontable_ext},
		{symbols.getSymbolId("clear"), clear_ext},
		{symbols.getSymbolId("handempty"), handempty_ext},
		{symbols.getSymbolId("holding"), holding_ext}});
}

CoreState generateGoalState(const SymbolTable& symbols) {
    auto on_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto ontable_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto clear_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto handempty_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());
	auto holding_ext = std::shared_ptr<PredicateExtension>(new PredicateExtension());

    clear_ext->set({symbols.get_object_id("d")}, true);
	
	on_ext->set({symbols.get_object_id("d"), symbols.get_object_id("c")}, true);
	on_ext->set({symbols.get_object_id("c"), symbols.get_object_id("b")}, true);
	on_ext->set({symbols.get_object_id("b"), symbols.get_object_id("a")}, true);
	ontable_ext->set({symbols.get_object_id("a")}, true);
	
	handempty_ext->set({}, true);
	
	return CoreState({{symbols.getSymbolId("on"), on_ext},
		{symbols.getSymbolId("ontable"), ontable_ext},
		{symbols.getSymbolId("clear"), clear_ext},
		{symbols.getSymbolId("handempty"), handempty_ext},
		{symbols.getSymbolId("holding"), holding_ext}});
}

void generate(aptk::core::Problem& problem) {

    /* Type declaration */
    TypeIdx object_t = problem.add_type("object", 0);

    /* Predicate and function declaration */
    problem.add_predicate("on", {object_t,object_t});
	problem.add_predicate("ontable", {object_t});
	problem.add_predicate("clear", {object_t});
	problem.add_predicate("handempty", {});
	problem.add_predicate("holding", {object_t});

    /* Object and constant declaration */
	problem.add_object("d", object_t);
	problem.add_object("b", object_t);
	problem.add_object("a", object_t);
	problem.add_object("c", object_t);

	const SymbolTable& symbols = problem.close_symbol_table();

	/* Parametrized actions */
	problem.add_parametrized_action(cActionPtr(new PickUpAction(symbols)));
	problem.add_parametrized_action(cActionPtr(new PutDownAction(symbols)));
	problem.add_parametrized_action(cActionPtr(new StackAction(symbols)));
	problem.add_parametrized_action(cActionPtr(new UnstackAction(symbols)));

	/* Bind the parametrized actions */
    problem.setActionBinder(std::unique_ptr<ActionBinder>(new ActionBinder(problem)));
	problem.bindActions();

	/* Define the initial state */
	problem.set_initial_state(generateInitialState(symbols));

	/* Define the goal evaluator */
	problem.setGoalEvaluator(cGoalEvaluatorPtr(new ConcreteGoalEvaluator(symbols)));
}

} } } } // namespaces