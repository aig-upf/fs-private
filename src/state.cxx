
#include <boost/functional/hash.hpp>

#include <state.hxx>
#include <problem_info.hxx>
#include <atom.hxx>


namespace fs0 {

State::State(unsigned numAtoms, const std::vector<Atom>& facts) :
	_bool_values(numAtoms), _int_values(numAtoms)
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
// 	_bool_values.at(atom.getVariable()) = value;
    if ( ProblemInfo::getInstance().getVariableGenericType(atom.getVariable()) == ProblemInfo::ObjectType::BOOL )
        _bool_values[atom.getVariable()] = value;
    else
        _int_values[atom.getVariable()] = value;
}

bool State::contains(const Atom& atom) const {
	return getValue(atom.getVariable()) == atom.getValue();
}

ObjectIdx
State::getValue(const VariableIdx& variable) const {
    return ProblemInfo::getInstance().getVariableGenericType(variable) == ProblemInfo::ObjectType::BOOL ? _bool_values[variable] : _int_values[variable];
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
	for (unsigned i = 0; i < _bool_values.size(); ++i) {
		if (info.getVariableGenericType(i) != ProblemInfo::ObjectType::BOOL) continue;
        if ( _bool_values[i] == 0 ) continue;
		os << info.getVariableName(i);
		if (i < _bool_values.size() - 1) os << ", ";
	}
    for (unsigned i = 0; i < _int_values.size(); ++i) {
        if (info.getVariableGenericType(i) == ProblemInfo::ObjectType::BOOL) continue;
        os << info.getVariableName(i) << "=" << info.getObjectName(i, _int_values[i]);
        if (i < _int_values.size() - 1) os << ", ";
    }

	os << "]";
	return os;
}


std::size_t State::computeHash() const {
	//return std::hash<BitsetT>{}(_bool_values);
    auto a = boost::hash_value( _bool_values);
    auto b = boost::hash_value( _int_values);
    return boost::hash_value( std::make_pair(a,b));
// 	return boost::hash_value(_bool_values);
}


} // namespaces
