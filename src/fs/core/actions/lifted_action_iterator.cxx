
#include <fs/core/state.hxx>
#include <fs/core/actions/lifted_action_iterator.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/constraints//gecode/handlers/lifted_action_csp.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/applicability/action_managers.hxx>

namespace fs0 { namespace gecode {

LiftedActionIterator::LiftedActionIterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const std::vector<const fs::Formula*>& state_constraints) :
	_handlers(handlers), _state(state), _state_constraints(state_constraints)
{}

LiftedActionIterator::Iterator::Iterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const std::vector<const fs::Formula*>& state_constraints, unsigned currentIdx) :
	_handlers(handlers),
	_state(state),
	_current_handler_idx(currentIdx),
	_engine(nullptr),
	_csp(nullptr),
	_action(nullptr),
	_state_constraints(state_constraints)
{
	advance();
}

LiftedActionIterator::Iterator::~Iterator() {
	if (_action) delete _action;
	if (_engine) delete _engine;
	if (_csp) delete _csp;
}

void LiftedActionIterator::Iterator::advance() {
	while (next_solution()) {

		// If there are no state constraints, the solution is necessarily valid
		if (_state_constraints.empty()) {
			return;
		}

		// Else, we need to check whether the application of the action that results from the CSP solution violates any state constraint
		// TODO - A better way to do this would be to integrate state constraints into the CSP
		std::unique_ptr<const GroundAction> ground(_action->generate());
		State next(_state, NaiveApplicabilityManager::computeEffects(_state, *ground));
        for ( auto c : _state_constraints ) {
    		if ( c->interpret(next) ) { // The application of the action would violate the state constraints
    			return;
    		}
        }
	}
}


bool LiftedActionIterator::Iterator::next_solution() {
	for (;_current_handler_idx < _handlers.size(); ++_current_handler_idx) {
		LiftedActionCSP& handler = *_handlers[_current_handler_idx];

		// std::cout << std::endl << "applicability CSP: " << handler << std::endl;

		if (!_csp) {
			_csp = handler.instantiate(_state);

			// std::cout << std::endl << "After instantiation: "; handler.print(std::cout, *_csp); std::cout << std::endl;

			if (!_csp || !_csp->checkConsistency()) { // The CSP is not even locally consistent, thus let's move to the next handler
				delete _csp; _csp = nullptr;
				continue;
			}
		}

		// We have a consistent CSP in '_csp'
		if (!_engine) {
			_engine = new engine_t(_csp);
		}

		// We have an instantiated engine in '_engine'
		GecodeCSP* solution = _engine->next();
		if (!solution) {
			delete _csp; _csp = nullptr;
			delete _engine; _engine = nullptr;
			continue; // The CSP is consistent but has no solution
		}

		if (_action) delete _action;
		_action = handler.get_lifted_action_id(solution);
		delete solution;
		break;
	}

	return _current_handler_idx != _handlers.size();
}
}} // namespaces
