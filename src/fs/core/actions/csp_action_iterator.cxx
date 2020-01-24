
#include <fs/core/state.hxx>
#include <fs/core/actions/csp_action_iterator.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/constraints/gecode/handlers/lifted_action_csp.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>

namespace fs0::gecode {

CSPActionIterator::CSPActionIterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const std::vector<const fs::Formula*>& state_constraints, const AtomIndex& tuple_index) :
        _handlers(handlers), _state(state), _state_constraints(state_constraints), _extension_handler(tuple_index, state)
{
}

CSPActionIterator::Iterator::Iterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const std::vector<const fs::Formula*>& state_constraints, const StateBasedExtensionHandler& extension_handler, unsigned currentIdx) :
        _handlers(handlers),
        _state(state),
        _current_handler_idx(currentIdx),
        _engine(nullptr),
        _csp(nullptr),
        _action(nullptr),
        _state_constraints(state_constraints),
        _extension_handler(extension_handler)
{
    advance();
}

CSPActionIterator::Iterator::~Iterator() {
    delete _action;
    delete _engine;
    delete _csp;
}

void CSPActionIterator::Iterator::advance() {
    while (next_solution()) {
        return;
        // If there are no state constraints, the solution is necessarily valid
        // TODO This is now mistaken, as now _state_constraints contains a single constraint which is a tautology,
        // but it's no longer empty ¿?¿?¿
        if (_state_constraints.empty()) {
            return;
        }
        std::cout << "STATE CONSTRAINTS " << *_state_constraints[0] << std::endl;
        assert(false);

        // Else, we need to check whether the application of the action that results from the CSP solution violates any state constraint
        // TODO - A better way to do this would be to integrate state constraints into the CSP
        std::unique_ptr<const GroundAction> ground(_action->generate());
        State next(_state, NaiveApplicabilityManager::computeEffects(_state, *ground));
        // TODO This is a bug, we need to check that ALL state constraints are satisfied before returning
        for ( auto c : _state_constraints ) {
            if ( c->interpret(next) ) { // The application of the action would violate the state constraints
                return;
            }
        }
    }
}


bool CSPActionIterator::Iterator::next_solution() {
    for (;_current_handler_idx < _handlers.size(); ++_current_handler_idx) {
        LiftedActionCSP& handler = *_handlers[_current_handler_idx];

        // std::cout << std::endl << "applicability CSP: " << handler << std::endl;

        if (!_csp) {
            _csp = handler.instantiate(_state, _extension_handler);

            // std::cout << std::endl << "After instantiation: "; handler.print(std::cout, *_csp); std::cout << std::endl;

            if (!_csp || !_csp->propagate()) { // The CSP is not even locally consistent, thus let's move to the next handler
                delete _csp; _csp = nullptr;
                continue;
            }
        }

        // We have a consistent CSP in '_csp'
        if (!_engine) {
            _engine = new engine_t(_csp);
        }

        // We have an instantiated engine in '_engine'
        GecodeSpace* solution = _engine->next();
        if (!solution) {
            delete _csp; _csp = nullptr;
            delete _engine; _engine = nullptr;
            continue; // The CSP is consistent but has no solution
        }

        delete _action;
        _action = handler.get_lifted_action_id(solution);
        delete solution;
        break;
    }

    return _current_handler_idx != _handlers.size();
}

} // namespaces
