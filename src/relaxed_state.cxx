
#include <relaxed_state.hxx>
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
	unsigned n = state.numAtoms();
	_domains.reserve(n);
	
	// For each vector index, we construct a new domain containing only the value from the non-relaxed state.
	for (unsigned i = 0; i < n; ++i) {
		DomainPtr domain = std::make_shared<Domain>();
		domain->insert(state.getValue(i));
		_domains.push_back(domain);
	}
}

void RelaxedState::accumulate(const std::vector<std::vector<object_id>>& atoms) {
	for (VariableIdx variable = 0; variable < atoms.size(); ++variable) {
		const auto& var_atoms = atoms[variable];
		_domains.at(variable)->insert(var_atoms.cbegin(), var_atoms.cend());
	}
}

std::ostream& RelaxedState::print(std::ostream& os) const {
	const ProblemInfo& problemInfo = ProblemInfo::getInstance();
	os << "RelaxedState[";
	for (unsigned i = 0; i < _domains.size(); ++i) { // Iterate through all the sets
		const DomainPtr& vals = _domains.at(i);
		assert(vals->size() != 0);
		
		os << problemInfo.getVariableName(i) << "={";
		for (const auto& obj:*vals) { // Iterate through the set elements.
			os << problemInfo.object_name(obj) << ",";
		}
		if (i < _domains.size() - 1) os << "}, ";
	}
	os << "]";
	return os;
}

} // namespaces
