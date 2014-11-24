
#pragma once

#include <iosfwd>
#include <core_types.hxx>
#include <actions.hxx>
#include <state.hxx>
#include <action_set_manager.hxx>

namespace aptk { namespace core {

template <class ActionManagerType>
class ApplicableActionSet {
protected:
	const ActionManagerType _actionManager;
	const ActionList& _boundActions;
	
public:
	ApplicableActionSet(const ActionManagerType& actionManager, const ActionList& boundActions) :
		_actionManager(actionManager),
		_boundActions(boundActions)
	{}
	
	friend std::ostream& operator<<(std::ostream &os, const ApplicableActionSet&  elem) { 
		return elem.print(os);
	}
	
	//! Prints a representation of the function to the given stream.
	std::ostream& print(std::ostream& os) const {
		os << "ApplicableActionSet[(total number of actions = " << _boundActions.size() << ")(actions: ";
		for (const auto& elem:_boundActions) {
			os << *elem << ",";
		}
		os << ")]";
		os.flush();
		return os;
	}
	
	class Iterator {
		friend class ApplicableActionSet;
		
	protected:
		Iterator(const ActionList& boundActions, const ActionManagerType& actionManager, unsigned currentIdx) :
			_boundActions(boundActions),
			_actionManager(actionManager),
			_currentIdx(currentIdx),
			_currentJustified()
		{
			advance();
		}

		const ActionList& _boundActions;
		const ActionManagerType& _actionManager;
		unsigned _currentIdx;
		JustifiedAction::ptr _currentJustified;
		
		void advance() {
			for (;_currentIdx != _boundActions.size(); ++_currentIdx) {
				_currentJustified = std::make_shared<JustifiedAction>(*_boundActions[_currentIdx]);
				if (_actionManager.isApplicable(*_currentJustified)) {
					break; // The current action is already applicable.
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

		std::pair<ActionIdx, JustifiedAction::ptr> operator*() const { return std::make_pair(_currentIdx, _currentJustified); }
		
		bool operator==(const Iterator &other) const { return _currentIdx == other._currentIdx; }
		bool operator!=(const Iterator &other) const { return !(this->operator==(other)); }
	};
	
	Iterator begin() const { return Iterator(_boundActions, _actionManager, 0); }
	Iterator end() const { return Iterator(_boundActions, _actionManager, _boundActions.size() ); }	
};

} } // namespaces
