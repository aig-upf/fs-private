
#include <fact.hxx>
#include <problem.hxx>

#include <state.hxx>

namespace fs0 {

	
std::ostream& Fact::print(std::ostream& os) const {
	auto problemInfo = Problem::getCurrentProblem()->getProblemInfo();
	os << "[" << problemInfo->getVariableName(_variable) << getSign() << problemInfo->getObjectName(_variable, _value) << "]";
	return os;
}
	
bool operator< (const Fact& lhs, const Fact& rhs) {
	if (lhs._variable < rhs._variable) return true;
	if (lhs._variable > rhs._variable) return false;
	if (lhs._value < rhs._value) return true;
	return false;
}

std::ostream& PlainAggregatedFact::print(std::ostream& os) const {
	os << "[";
	for (std::vector<Fact::cptr>::const_iterator fact = elements.cbegin();;) {
 		os << **fact;
		++fact;
		if (fact != elements.cend()) os << " " + getOperatorName() + " ";
		else break;
	}
	
	os << "]";
	return os;
}


// bool Fact::isSatisfiable(const State& state) const {
// 	return ActionManager::checkFormulaSatisfiable(*this, state);
// }
// 
// bool NegatedFact::isSatisfiable(const State& state) const {
// 	return ActionManager::checkFormulaSatisfiable(*this, state);
// }

} // namespaces
