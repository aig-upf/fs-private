
#include <fs/core/state.hxx>
#include <fs/core/actions/csp_action_iterator.hxx>
#include <fs/core/actions/action_id.hxx>
#include <fs/core/constraints/gecode/handlers/lifted_action_csp.hxx>
#include <fs/core/constraints/gecode/v2/gecode_space.hxx>

namespace fs0::gecode {

CSPActionIterator::CSPActionIterator(
        const State& state,
        const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers,
        const std::vector<v2::ActionSchemaCSP>& handlers2,
        const std::vector<unsigned>& symbols_in_extensions,
        const std::vector<const PartiallyGroundedAction*>& schemas,
        const AtomIndex& tuple_index) :
        _handlers(handlers),
        handlers2(handlers2),
        schemas(schemas),
        _state(state),
        _extension_handler(tuple_index, symbols_in_extensions, state)
{
}

CSPActionIterator::Iterator::Iterator(
        const State& state,
        const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers,
        const std::vector<v2::ActionSchemaCSP>& handlers2,
        const std::vector<const PartiallyGroundedAction*>& schemas,
        const StateBasedExtensionHandler& extension_handler, unsigned currentIdx) :

        _handlers(handlers),
        handlers2(handlers2),
        schemas(schemas),
        num_handlers(handlers2.size()),
        _state(state),
        _current_handler_idx(currentIdx),
        _engine(nullptr),
        _csp(nullptr),
        _action(nullptr),
        _extension_handler(extension_handler)
{
    assert(_handlers.size() == num_handlers);
    assert(schemas.size() == num_handlers);
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
    }
}


bool CSPActionIterator::Iterator::next_solution() {
    for (;_current_handler_idx < num_handlers; ++_current_handler_idx) {
//        LiftedActionCSP& handler = *_handlers[_current_handler_idx];
        const v2::ActionSchemaCSP& handler2 = handlers2[_current_handler_idx];

        // std::cout << std::endl << "applicability CSP: " << handler << std::endl;

        if (!_csp) {
//            _csp = handler.instantiate(_state, _extension_handler);
            _csp = handler2.instantiate(_state, _extension_handler);

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
        auto solution = _engine->next();
        if (!solution) {
            delete _csp; _csp = nullptr;
            delete _engine; _engine = nullptr;
            continue; // The CSP is consistent but has no solution
        }

        delete _action;
//        _action = handler.get_lifted_action_id(solution);
        _action = new LiftedActionID(schemas[_current_handler_idx], handler2.build_binding_from_solution(solution));
        delete solution;
        break;
    }

    return _current_handler_idx != _handlers.size();
}

} // namespaces
