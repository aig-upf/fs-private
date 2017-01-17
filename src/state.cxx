
#include <boost/functional/hash.hpp>

#include <state.hxx>
#include <problem_info.hxx>
#include <atom.hxx>


namespace fs0 {

State::State(unsigned numAtoms, const std::vector<Atom>& facts) :
	_values(numAtoms)
{
	// Note that those facts not explicitly set in the initial state will be initialized to 0, i.e. "false", which is convenient to us.
	for (const auto& fact:facts) { // Insert all the elements of the vector
		set(fact);
	}
	updateHash();
}

State::State(const State& state, const std::vector<Atom>& atoms) :
	State(state) {
	accumulate(atoms);
}
	
void State::set(const Atom& atom) {
	auto value = atom.getValue();
// 	_values.at(atom.getVariable()) = value;
	_values[atom.getVariable()] = value;
}

bool State::contains(const Atom& atom) const {
	return getValue(atom.getVariable()) == atom.getValue();
}

ObjectIdx State::getValue(const VariableIdx& variable) const {
// 	return _values.at(variable);
	return _values[variable];
}

//! Applies the given changeset into the current state.
void State::accumulate(const std::vector<Atom>& atoms) {
	for (const Atom& fact:atoms) { 
		set(fact);
	}
	updateHash(); // Important to update the hash value after all the changes have been applied!
}

std::ostream& State::print(std::ostream& os) const {
	const ProblemInfo& info = ProblemInfo::getInstance();
	os << "State";
	os << "(" << _hash << ")[";
	for (unsigned i = 0; i < _values.size(); ++i) {
		if (info.getVariableGenericType(i) == ProblemInfo::ObjectType::BOOL) {
			if (_values[i] == 0) continue;
			
			// Print only those atoms which are true in the state
			os << info.getVariableName(i);
		} else {
			os << info.getVariableName(i) << "=" << info.getObjectName(i, _values[i]);
		}
		if (i < _values.size() - 1) os << ", ";
	}
	os << "]";
	return os;
}


std::size_t State::computeHash() const { 
	return boost::hash_value(_values);
	//return boost::hash_range(_values.begin(), _values.end());
}


} // namespaces
