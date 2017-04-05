
#pragma once

#include <memory>

#include <gecode/driver.hh>

namespace fs0 {
class State;
class LiftedActionID;
}

namespace fs0 { namespace language { namespace fstrips { class Formula; } }}
namespace fs = fs0::language::fstrips;

namespace fs0 { namespace gecode {

class GecodeCSP;
class LiftedActionCSP;

//! An iterator that models action schema applicability as an action CSP.
//! The iterator receives an (ordered) set of lifted-action CSP handlers, and upon iteration
//! returns, chainedly, each of the lifted-action IDs that are applicable.
class LiftedActionIterator {
protected:
	const std::vector<std::shared_ptr<LiftedActionCSP>>& _handlers;
	
	const State& _state;
	
	const fs::Formula* _state_constraints;
	
public:
	LiftedActionIterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const fs::Formula* state_constraints);
	
	class Iterator {
		friend class LiftedActionIterator;
		
	public:
		using engine_t = Gecode::DFS<GecodeCSP>;
		
		~Iterator();
		
	protected:
		Iterator(const State& state, const std::vector<std::shared_ptr<LiftedActionCSP>>& handlers, const fs::Formula* state_constraints, unsigned currentIdx);

		const std::vector<std::shared_ptr<LiftedActionCSP>>& _handlers;
		
		const State& _state;
		
		unsigned _current_handler_idx;
		
		engine_t* _engine;
		
		GecodeCSP* _csp;
		
		LiftedActionID* _action;
		
		//! The state constraints
		const fs::Formula* _state_constraints;
		
		void advance();
		
		//! Returns true iff a new solution has actually been found
		bool next_solution();

	public:
		const Iterator& operator++() {
			advance();
			return *this;
		}
		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		const LiftedActionID& operator*() const { return *_action; }
		
		//! This is not really true... but will work for the purpose of comparing with the end iterator.
		bool operator==(const Iterator &other) const { return _current_handler_idx == other._current_handler_idx; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
	Iterator begin() const { return Iterator(_state, _handlers, _state_constraints, 0); }
	Iterator end() const { return Iterator(_state,_handlers, _state_constraints, _handlers.size()); }
};


} } // namespaces
