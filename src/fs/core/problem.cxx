
#include <fs/core/problem.hxx>

#include <fs/core/actions/actions.hxx>
#include <fs/core/applicability/formula_interpreter.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/metrics.hxx>
#include <fs/core/languages/fstrips/operations/axioms.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/state.hxx>
#include <fs/core/utils/utils.hxx>

#include <utility>


namespace fs0 {

std::unique_ptr<Problem> Problem::_instance = nullptr;

Problem::Problem(
        State* init,
        StateAtomIndexer* state_indexer,
        std::vector<const ActionData*> action_data,
        std::unordered_map<std::string, const fs::Axiom*> axioms,
        const fs::Formula* goal,
        std::unordered_map<std::string, const fs::Axiom*> state_constraints,
        const fs::Metric* metric,
        AtomIndex&& tuple_index,
        AllTransitionGraphsT transitions
) :
	_tuple_index(std::move(tuple_index)),
	_init(init),
	_state_indexer(state_indexer),
	_action_data(std::move(action_data)),
	_axioms(std::move(axioms)),
	_ground(),
	_partials(),
	_state_constraints(std::move(state_constraints)),
	_goal_formula(goal),
    _metric(metric),
	_goal_sat_manager(FormulaInterpreter::create(_goal_formula, get_tuple_index())),
	_is_predicative(check_is_predicative()),
    _transition_graphs(std::move(transitions))
{
    //! Store pointers to the state constraint definitions for ease of use
    for ( auto c : _state_constraints ) {
        _state_constraints_formulae.push_back( c.second->getDefinition() );
    }
}

Problem::~Problem() {
	for (const auto pointer:_action_data) delete pointer;
	for (const auto& it:_axioms) delete it.second;
	for (const auto& it:_state_constraints) delete it.second;
	for (const auto pointer:_ground) delete pointer;
	for (const auto pointer:_partials) delete pointer;
	delete _goal_formula;
	delete _metric;
}

std::unordered_map<std::string, const fs::Axiom*>
_clone_axioms(const std::unordered_map<std::string, const fs::Axiom*>& axioms) {
	std::unordered_map<std::string, const fs::Axiom*> cloned;
	for (const auto& it:axioms) {
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
    _state_constraints(other._state_constraints),
	_goal_formula(other._goal_formula->clone()),
    _metric(new fs::Metric(*other._metric)),
	_goal_sat_manager(other._goal_sat_manager->clone()),
	_is_predicative(other._is_predicative)
{
    //! Store pointers to the state constraint definitions for ease of use
    for ( auto c : _state_constraints ) {
        _state_constraints_formulae.push_back( c.second->getDefinition() );
    }
}
const fs::Formula* Problem::getGoalConditions() const { return _goal_formula; }

std::ostream& Problem::print(std::ostream& os) const {
	const fs0::ProblemInfo& info = ProblemInfo::getInstance();
	os << "Planning Problem [domain: " << info.getDomainName() << ", instance: " << info.getInstanceName() <<  "]" << std::endl;

	os << "Goal Conditions:" << std::endl << "------------------" << std::endl;
	os << "\t" << *getGoalConditions() << std::endl;
	os << std::endl;

	os << "State Constraints:" << std::endl << "------------------" << std::endl;
	for ( auto c : _state_constraints ) {
	os << "\t" << c.first << ": " << *(c.second) << std::endl;
	}

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

} // namespaces
