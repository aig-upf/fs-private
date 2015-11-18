
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <actions/action_id.hxx>
#include <constraints/gecode/handlers/action_schema_handler.hxx>
#include <utils/logging.hxx>

#include <gecode/driver.hh>

namespace fs0 { namespace gecode {

//! An iterator that models action schema applicability as an action CSP.
//! The iterator receives an (ordered) set of lifted-action CSP handlers, and upon iteration
//! returns, chainedly, each of the lifted-action IDs that are applicable.
class LiftedActionIterator {
protected:
	const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& _handlers;
	
	const State& _state;
	
public:
	LiftedActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& handlers) :
		_handlers(handlers), _state(state)
	{}
	
	class Iterator {
		friend class LiftedActionIterator;
		
	public:
		typedef Gecode::DFS<SimpleCSP> engine_t;
		// typedef std::pair<LiftedActionID*, State*> element_t;
		
		~Iterator() {
			if (_element) delete _element;
		}
		
	protected:
		Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& handlers, unsigned currentIdx) :
			_handlers(handlers),
			_state(state),
			_current_handler_idx(currentIdx),
			_engine(nullptr),
			_csp(nullptr),
			_element(nullptr)
		{
			advance();
		}
		


		const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& _handlers;
		
		const State& _state;
		
		unsigned _current_handler_idx;
		
		engine_t* _engine;
		
		SimpleCSP* _csp;
		
		LiftedActionID* _element;
		
		void advance() {
			for (;_current_handler_idx != _handlers.size(); ++_current_handler_idx) {
				ActionSchemaCSPHandler& handler = *_handlers[_current_handler_idx];
				
				if (!_csp) {
					_csp = handler.instantiate_csp(_state);
					
					if (!_csp->checkConsistency()) { // The CSP is not even locally consistent, thus let's move to the next handler
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

	public:
		const Iterator& operator++() {
			advance();
			return *this;
		}
		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		const LiftedActionID& operator*() const { return *_element; }
		
		//! This is not really true... but will work for the purpose of comparing with the end iterator.
		bool operator==(const Iterator &other) const { return _current_handler_idx == other._current_handler_idx; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
	Iterator begin() const { return Iterator(_state, _handlers, 0); }
	Iterator end() const { return Iterator(_state,_handlers, _handlers.size()); }
};


} } // namespaces
