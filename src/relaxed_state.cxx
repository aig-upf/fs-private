
#include <relaxed_state.hxx>
#include <problem.hxx>
#include <problem_info.hxx>
#include <state.hxx>

namespace fs0 {
	

RelaxedState::~RelaxedState() {
	assert(checkPointerOwnershipIsCorrect());
}

bool RelaxedState::checkPointerOwnershipIsCorrect() const {
	for (const DomainPtr& domain:_domains) {
		if (domain.use_count() != 1) return false;
	}
	return true;
}

RelaxedState::RelaxedState(const State& state) {
	_domains.reserve(state.numAtoms());
	
	// For each vector index, we construct a new domain containing only the value from the non-relaxed state.
	for (ObjectIdx value:state.getValues()) {
		DomainPtr domain = std::make_shared<Domain>();
		domain->insert(value);
		_domains.push_back(domain);
	}
}

std::ostream& RelaxedState::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = Problem::getInfo();
	os << "RelaxedState[";
	for (unsigned i = 0; i < _domains.size(); ++i) { // Iterate through all the sets
		const DomainPtr& vals = _domains.at(i);
		assert(vals->size() != 0);
		
		os << problemInfo.getVariableName(i) << "={";
		for (const auto& objIdx:*vals) { // Iterate through the set elements.
			os << problemInfo.getObjectName(i, objIdx) << ",";
		}
		if (i < _domains.size() - 1) os << "}, ";
	}
	os << "]";
	return os;
}

} // namespaces
