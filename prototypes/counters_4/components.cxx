
#include "components.hxx"
#include <utils/loader.hxx>
#include <typeinfo>
#include <constraints/gecode/translator_repository.hxx>

std::unique_ptr<External> external;

const std::string IncrementAction::name_ = "increment";
const ActionSignature IncrementAction::signature_ = {2};
const std::string DecrementAction::name_ = "decrement";
const ActionSignature DecrementAction::signature_ = {2};


void generate(const std::string& data_dir, Problem& problem) {
	
	fs0::gecode::TranslatorRepository::instance().addEntry(typeid(IncrementActionEffect0),   new IncrementActionEffect0Implementer());
	fs0::gecode::TranslatorRepository::instance().addEntry(typeid(DecrementActionEffect0),   new DecrementActionEffect0Implementer());
	
	external = std::unique_ptr<External>(new External(problem.getProblemInfo(), data_dir));
	Loader::loadProblem(data_dir, ComponentFactory::instantiateAction, ComponentFactory::instantiateConstraint, ComponentFactory::instantiateGoal, problem);
}
