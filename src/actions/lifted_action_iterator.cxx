
#include <actions/lifted_action_iterator.hxx>
#include <actions/action_id.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>

namespace fs0 { namespace gecode {

LiftedActionIterator::LiftedActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& handlers) :
	_handlers(handlers), _state(state)
{}

LiftedActionIterator::Iterator::Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& handlers, unsigned currentIdx) :
	_handlers(handlers),
	_state(state),
	_current_handler_idx(currentIdx),
	_engine(nullptr),
	_csp(nullptr),
	_element(nullptr)
{
	advance();
}

LiftedActionIterator::Iterator::~Iterator() {
	if (_element) delete _element;
}

void LiftedActionIterator::Iterator::advance() {
	for (;_current_handler_idx != _handlers.size(); ++_current_handler_idx) {
		ActionSchemaCSPHandler& handler = *_handlers[_current_handler_idx];
		
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
		SimpleCSP* solution = _engine->next();
		if (!solution) {
			delete _csp; _csp = nullptr;
			delete _engine; _engine = nullptr;
			continue; // The CSP is consistent but has no solution
		}
		
		if (_element) delete _element;
		_element = handler.get_lifted_action_id(solution);
		delete solution;
		break;
	}
}

}} // namespaces
