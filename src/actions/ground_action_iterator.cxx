
#include <actions/ground_action_iterator.hxx>

namespace fs0 {

GroundActionIterator::GroundActionIterator(const ApplicabilityManager& actionManager, const State& state, const std::vector<const GroundAction*>& actions) :
	_actionManager(actionManager), _actions(actions), _state(state)
{}
	
GroundActionIterator::Iterator::Iterator(const State& state, const std::vector<const GroundAction*>& actions, const ApplicabilityManager& actionManager, unsigned currentIdx) :
	_actionManager(actionManager),
	_actions(actions),
	_state(state),
	_currentIdx(currentIdx)
{
	advance();
}

void GroundActionIterator::Iterator::advance() {
	for (;_currentIdx != _actions.size(); ++_currentIdx) {
		if (_actionManager.isApplicable(_state, *_actions[_currentIdx])) { // The action is applicable, break the for loop.
			break;
		}
	}
}

const GroundActionIterator::Iterator& GroundActionIterator::Iterator::operator++() {
	++_currentIdx;
	advance();
	return *this;
}

} // namespaces
