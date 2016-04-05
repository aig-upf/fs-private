
#pragma once

#include <fs0_types.hxx>
#include <state.hxx>
#include <applicability/applicability_manager.hxx>
#include <actions/actions.hxx>

namespace fs0 {


/**
 * A simple iterator strategy to iterate over the actions applicable in a given state.
 */
class ApplicableActionSet {
protected:
	const ApplicabilityManager _actionManager;
	
	const std::vector<GroundAction::cptr>& _actions;
	
	const State& _state;
	
public:
	ApplicableActionSet(const ApplicabilityManager& actionManager, const State& state, const std::vector<GroundAction::cptr>& actions) :
		_actionManager(actionManager), _actions(actions), _state(state)
	{}
	
	class Iterator {
		friend class ApplicableActionSet;
		
	protected:
		Iterator(const State& state, const std::vector<GroundAction::cptr>& actions, const ApplicabilityManager& actionManager, unsigned currentIdx) :
			_actionManager(actionManager),
			_actions(actions),
			_state(state),
			_currentIdx(currentIdx)
		{
			advance();
		}

		const ApplicabilityManager& _actionManager;
		
		const std::vector<GroundAction::cptr>& _actions;
		
		const State& _state;
		
		unsigned _currentIdx;
		
		void advance() {
			for (;_currentIdx != _actions.size(); ++_currentIdx) {
				if (_actionManager.isApplicable(_state, *_actions[_currentIdx])) { // The action is applicable, break the for loop.
					break;
				}
			}
		}

	public:
		const Iterator& operator++() {
			++_currentIdx;
			advance();
			return *this;
		}
		const Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}

		ActionIdx operator*() const { return _currentIdx; }
		
		bool operator==(const Iterator &other) const { return _currentIdx == other._currentIdx; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
	Iterator begin() const { return Iterator(_state, _actions, _actionManager, 0); }
	Iterator end() const { return Iterator(_state,_actions, _actionManager, _actions.size() ); }
};


} // namespaces
