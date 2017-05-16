
#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>
#include <actions/actions.hxx>
#include "actions/grounding.hxx"
#include <lapkt/tools/logging.hxx>
#include <utils/printers/actions.hxx>
#include <utils/utils.hxx>
#include <applicability/formula_interpreter.hxx>
#include <languages/fstrips/formulae.hxx>
#include <languages/fstrips/axioms.hxx>
#include <languages/fstrips/operations/axioms.hxx>


namespace fs0 {

std::unique_ptr<Problem> Problem::_instance = nullptr;

Problem::Problem(State* init, StateAtomIndexer* state_indexer, const std::vector<const ActionData*>& action_data, const std::unordered_map<std::string, const fs::Axiom*>& axioms, const fs::Formula* goal, const fs::Formula* state_constraints, AtomIndex&& tuple_index) :
	_tuple_index(std::move(tuple_index)),
	_init(init),
	_state_indexer(state_indexer),
	_action_data(action_data),
	_axioms(axioms),
	_ground(),
	_partials(),
	_state_constraint_formula(state_constraints),
	_goal_formula(goal),
	_goal_sat_manager(FormulaInterpreter::create(_goal_formula, get_tuple_index())),
	_is_predicative(check_is_predicative())
{
}

Problem::~Problem() {
	for (const auto pointer:_action_data) delete pointer;
	for (const auto it:_axioms) delete it.second;
	for (const auto pointer:_ground) delete pointer;
	for (const auto pointer:_partials) delete pointer;
	delete _state_constraint_formula;
	delete _goal_formula;
}

std::unordered_map<std::string, const fs::Axiom*>
_clone_axioms(const std::unordered_map<std::string, const fs::Axiom*>& axioms) {
	std::unordered_map<std::string, const fs::Axiom*> cloned;
	for (const auto it:axioms) {
		cloned.insert(std::make_pair(it.first, new fs::Axiom(*it.second)));
	}
	return cloned;
}

Problem::Problem(const Problem& other) :
	_tuple_index(other._tuple_index),
	_init(new State(*other._init)),
	_state_indexer(new StateAtomIndexer(*other._state_indexer)),
	_action_data(Utils::copy(other._action_data)),
	_axioms(other._axioms),
	_ground(Utils::copy(other._ground)),
	_partials(Utils::copy(other._partials)),
	_state_constraint_formula(other._state_constraint_formula->clone()),
	_goal_formula(other._goal_formula->clone()),
	_goal_sat_manager(other._goal_sat_manager->clone()),
	_is_predicative(other._is_predicative)
{}

void Problem::set_state_constraints(const fs::Formula* state_constraint_formula) {
	delete _state_constraint_formula;
	_state_constraint_formula = state_constraint_formula;
}

void Problem::set_goal(const fs::Formula* goal) {
	delete _goal_formula;
	_goal_formula = goal;
}

std::ostream& Problem::print(std::ostream& os) const {
	const fs0::ProblemInfo& info = ProblemInfo::getInstance();
	os << "Planning Problem [domain: " << info.getDomainName() << ", instance: " << info.getInstanceName() <<  "]" << std::endl;

	os << "Goal Conditions:" << std::endl << "------------------" << std::endl;
	os << "\t" << *getGoalConditions() << std::endl;
	os << std::endl;

	os << "State Constraints:" << std::endl << "------------------" << std::endl;
	os << "\t" << *getStateConstraints() << std::endl;
	os << std::endl;

	os << "Action data" << std::endl << "------------------" << std::endl;
	for (const ActionData* data:_action_data) {
		os << print::action_data(*data) << std::endl;
	}
	os << std::endl;

	os << "Ground Actions: " << _ground.size();
	// os << std::endl << "------------------" << std::endl;
	// for (const const GroundAction* elem:_ground) {
	// 	os << *elem << std::endl;
	// }
	os << std::endl;

	return os;
}

bool Problem::check_is_predicative() {
	const ProblemInfo& info = ProblemInfo::getInstance();
	for (unsigned symbol = 0; symbol < info.getNumLogicalSymbols(); ++symbol) {
		if (!info.isPredicate(symbol)) return false;
	}
	return true;
}

//! TODO This is very hackyish. What we want to solve with this is the circular dependency
//! between loading the actions and loading the axioms... we cannot properly identify
//! as axioms those symbols which are axioms until all axioms have been loaded. Thus, we
//! currently load them as static user-defined procedures, and afterwards invoke this method
//! to replace them by axioms.
//! This needs to be called before grounding actions.
void Problem::consolidateAxioms() {
	const ProblemInfo& info = ProblemInfo::getInstance();

	// NOTE Order is FUNDAMENTAL: we need to process the axioms first of all.
	// TODO This won't work for recursive axioms. We need a better strategy, e.g. lazy retrieval of the axiom pointers whenever interpretation is required, etc.
	// Update the axioms
	for (auto& it:_axioms) {
		const fs::Axiom* axiom = it.second;
		auto definition = fs::process_axioms(*(axiom->getDefinition()), info);
		it.second = new fs::Axiom(axiom->getName(), axiom->getSignature(), axiom->getParameterNames(), axiom->getBindingUnit(), definition);
		delete axiom;
	}

	auto tmp = _goal_formula;
	_goal_formula = fs::process_axioms(*_goal_formula, info);
	delete tmp;
    auto old_manager = _goal_sat_manager.release();

	_goal_sat_manager = std::unique_ptr<FormulaInterpreter>(FormulaInterpreter::create(_goal_formula, get_tuple_index()));

	tmp = _state_constraint_formula;
	_state_constraint_formula = fs::process_axioms(*_state_constraint_formula, info);
	delete tmp;

	// Update the action schemas
	std::vector<const ActionData*> processed_actions;
	for (const ActionData* data:_action_data) {
		auto precondition = fs::process_axioms(*(data->getPrecondition()), info);

		std::vector<const fs::ActionEffect*> effects;
		for (const fs::ActionEffect* effect:data->getEffects()) {
			effects.push_back(fs::process_axioms(*effect, info));
		}

		processed_actions.push_back(new ActionData(data->getId(), data->getName(), data->getSignature(), data->getParameterNames(), data->getBindingUnit(), precondition, effects));
		delete data;
	}
	_action_data = processed_actions;
}



} // namespaces
