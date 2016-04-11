
#include <actions/applicable_action_set.hxx>

namespace fs0 {

ApplicableActionSet::ApplicableActionSet(const ApplicabilityManager& actionManager, const State& state, const std::vector<const GroundAction*>& actions) :
	_actionManager(actionManager), _actions(actions), _state(state)
{}
	
ApplicableActionSet::Iterator::Iterator(const State& state, const std::vector<const GroundAction*>& actions, const ApplicabilityManager& actionManager, unsigned currentIdx) :
	_actionManager(actionManager),
	_actions(actions),
	_state(state),
	_currentIdx(currentIdx)
{
	advance();
}

void ApplicableActionSet::Iterator::advance() {
	for (;_currentIdx != _actions.size(); ++_currentIdx) {
		if (_actionManager.isApplicable(_state, *_actions[_currentIdx])) { // The action is applicable, break the for loop.
			break;
		}
	}
}

const ApplicableActionSet::Iterator& ApplicableActionSet::Iterator::operator++() {
	++_currentIdx;
	advance();
	return *this;
}

} // namespaces
