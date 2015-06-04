
#include "components.hxx"
#include <utils/loader.hxx>
#include <typeinfo>
#include <constraints/gecode/expr_translator_repository.hxx>

std::unique_ptr<External> external;

const std::string IncrementAction::name_ = "increment";
const ActionSignature IncrementAction::signature_ = {2};
const std::string DecrementAction::name_ = "decrement";
const ActionSignature DecrementAction::signature_ = {2};

fs0::gecode::TranslatorRegistrar< IncrementActionEffect0Implementer > registrarIncrementEffect0( typeid( IncrementActionEffect0 ) );
fs0::gecode::TranslatorRegistrar< DecrementActionEffect0Implementer > registrarDecrementEffect0( typeid( DecrementActionEffect0 ) );

void generate(const std::string& data_dir, Problem& problem) {
	external = std::unique_ptr<External>(new External(problem.getProblemInfo(), data_dir));
	Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateConstraint, ComponentFactory::instantiateGoal, problem);
}
