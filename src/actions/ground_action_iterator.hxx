
#pragma once

#include <applicability/applicability_manager.hxx>

namespace fs0 {

class State;
class GroundAction;

//! A simple iterator strategy to iterate over the actions applicable in a given state.
class GroundActionIterator {
protected:
	const ApplicabilityManager _actionManager;
	
	const std::vector<const GroundAction*>& _actions;
	
	const State& _state;
	
public:
	GroundActionIterator(const ApplicabilityManager& actionManager, const State& state, const std::vector<const GroundAction*>& actions);
	
	class Iterator {
		friend class GroundActionIterator;
		
	protected:
		Iterator(const State& state, const std::vector<const GroundAction*>& actions, const ApplicabilityManager& actionManager, unsigned currentIdx);

		const ApplicabilityManager& _actionManager;
		
		const std::vector<const GroundAction*>& _actions;
		
		const State& _state;
		
		unsigned _currentIdx;
		
		void advance();

	public:
		const Iterator& operator++();
		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		ActionIdx operator*() const { return _currentIdx; }
		
		bool operator==(const Iterator &other) const { return _currentIdx == other._currentIdx; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
	Iterator begin() const { return Iterator(_state, _actions, _actionManager, 0); }
	Iterator end() const { return Iterator(_state,_actions, _actionManager, _actions.size() ); }
};


} // namespaces
