
#pragma once

#include <memory>

#include <gecode/driver.hh>

namespace fs0 {
	
class State;
class LiftedActionID;
}

namespace fs0 { namespace gecode {

class SimpleCSP;
class ActionSchemaCSPHandler;

//! An iterator that models action schema applicability as an action CSP.
//! The iterator receives an (ordered) set of lifted-action CSP handlers, and upon iteration
//! returns, chainedly, each of the lifted-action IDs that are applicable.
class LiftedActionIterator {
protected:
	const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& _handlers;
	
	const State& _state;
	
public:
	LiftedActionIterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& handlers);
	
	class Iterator {
		friend class LiftedActionIterator;
		
	public:
		typedef Gecode::DFS<SimpleCSP> engine_t;
		
		~Iterator();
		
	protected:
		Iterator(const State& state, const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& handlers, unsigned currentIdx);

		const std::vector<std::shared_ptr<ActionSchemaCSPHandler>>& _handlers;
		
		const State& _state;
		
		unsigned _current_handler_idx;
		
		engine_t* _engine;
		
		SimpleCSP* _csp;
		
		LiftedActionID* _element;
		
		void advance();

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
