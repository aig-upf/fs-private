
#include "csp_action_iterator.hxx"

#include <fs/core/actions/action_id.hxx>
#include <fs/core/constraints/gecode/v2/gecode_space.hxx>
#include <fs/core/state.hxx>


namespace fs0::gecode {

CSPActionIterator::CSPActionIterator(
        const State& state,
        const std::vector<v2::ActionSchemaCSP>& schema_csps,
        std::vector<Gecode::TupleSet>&& symbol_extensions,
        const std::vector<const PartiallyGroundedAction*>& schemas) :

    schema_csps(schema_csps),
    schemas(schemas),
    _state(state),
    symbol_extensions(std::move(symbol_extensions))
{
}

CSPActionIterator::Iterator::Iterator(
        const State& state,
        const std::vector<v2::ActionSchemaCSP>& schema_csps,
        const std::vector<const PartiallyGroundedAction*>& schemas,
        const std::vector<Gecode::TupleSet>& symbol_extensions,
        unsigned currentIdx) :

    schema_csps(schema_csps),
    schemas(schemas),
    num_schema_csps(schema_csps.size()),
    _state(state),
    _current_handler_idx(currentIdx),
    _engine(nullptr),
    _csp(nullptr),
    _action(nullptr),
    symbol_extensions(symbol_extensions)
{
    assert(schemas.size() == num_schema_csps);
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
    for (; _current_handler_idx < num_schema_csps; ++_current_handler_idx) {
        const v2::ActionSchemaCSP& schema_csp = schema_csps.at(_current_handler_idx);

        // std::cout << std::endl << "applicability CSP: " << handler << std::endl;

        if (!_csp) {
            _csp = schema_csp.instantiate(_state, symbol_extensions);

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
        auto* solution = _engine->next();
        if (!solution) {
            delete _csp; _csp = nullptr;
            delete _engine; _engine = nullptr;
            continue; // The CSP is consistent but has no solution
        }

        delete _action;
//        _action = handler.get_lifted_action_id(solution);
        _action = new LiftedActionID(schemas[_current_handler_idx], schema_csp.build_binding_from_solution(solution));
        delete solution;
        break;
    }

    return _current_handler_idx != num_schema_csps;
}

} // namespaces
