
#pragma once

#include <iosfwd>
#include <fs0_types.hxx>
#include <state.hxx>
#include <standard_applicability_manager.hxx>

namespace fs0 {


/**
 * A simple iterator strategy to iterate over the actions applicable in a given state.
 */
class SimpleApplicableActionSet {
protected:
	const StandardApplicabilityManager _actionManager;
	
	const Action::vcptr& _actions;
	
public:
	SimpleApplicableActionSet(const StandardApplicabilityManager& actionManager, const Action::vcptr& actions) :
		_actionManager(actionManager), _actions(actions)
	{}
	
	class Iterator {
		friend class SimpleApplicableActionSet;
		
	protected:
		Iterator(const Action::vcptr& actions, const StandardApplicabilityManager& actionManager, unsigned currentIdx) :
			_actions(actions),
			_actionManager(actionManager),
			_currentIdx(currentIdx)
		{
			advance();
		}

		const Action::vcptr& _actions;
		
		const StandardApplicabilityManager _actionManager;
		
		unsigned _currentIdx;
		
		void advance() {
			for (;_currentIdx != _actions.size(); ++_currentIdx) {
				if (_actionManager.isApplicable(*_actions[_currentIdx])) { // The action is applicable, break the for loop.
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
	
	Iterator begin() const { return Iterator(_actions, _actionManager, 0); }
	Iterator end() const { return Iterator(_actions, _actionManager, _actions.size() ); }
};


} // namespaces
