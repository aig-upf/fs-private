
#include <boost/functional/hash.hpp>

#include <state.hxx>
#include <problem_info.hxx>
#include <atom.hxx>


namespace fs0 {


StateAtomIndexer*
StateAtomIndexer::create(const ProblemInfo& info)
{
	unsigned n_vars = info.getNumVariables(), n_bool = 0, n_int = 0;
	IndexT index;
	index.reserve(n_vars);
	for (unsigned var = 0; var < n_vars; ++var) {
		if (info.isPredicativeVariable(var)) {
			index.push_back(std::make_pair(true, n_bool++)); // Important to post-increment
		} else {
			index.push_back(std::make_pair(false, n_int++)); // Important to post-increment
		}
	}
	assert(index.size() == n_vars && n_vars == n_bool + n_int);

	return new StateAtomIndexer(std::move(index), n_bool, n_int);
}

StateAtomIndexer::StateAtomIndexer(IndexT&& index, unsigned n_bool, unsigned n_int) :
	_index(std::move(index)), _n_bool(n_bool), _n_int(n_int)
{
}

StateAtomIndexer::IndexT
StateAtomIndexer::compute_index(const ProblemInfo& info) {
	unsigned n_vars = info.getNumVariables(), n_bool = 0, n_int = 0;
	IndexT index;
	index.reserve(n_vars);
	for (unsigned var = 0; var < n_vars; ++var) {
		if (info.isPredicativeVariable(var)) {
			index.push_back(std::make_pair(true, n_bool++)); // Important to post-increment
		} else {
			index.push_back(std::make_pair(false, n_int++)); // Important to post-increment
		}
	}
	assert(index.size() == n_vars && n_vars == n_bool + n_int);
	return index;
}

object_id
StateAtomIndexer::get(const State& state, VariableIdx variable) const {
	std::size_t n_vars = _index.size();
	assert(variable < n_vars);

	// If the state is fully boolean or fully multivalued, we can optimize the operation,
	// since the variable index will be exactly `variable`
	if (n_vars == _n_bool) return make_object(state._bool_values[variable]);
	if (n_vars == _n_int) return state._int_values[variable];

	// Otherwise we need to deindex the variable
	const IndexElemT& ind = _index[variable];
	if (ind.first) return make_object(state._bool_values[ind.second]);
	else return state._int_values[ind.second];
}

void
StateAtomIndexer::set(State& state, const Atom& atom) const { set(state, atom.getVariable(), atom.getValue()); }

void
StateAtomIndexer::set(State& state, VariableIdx variable, const object_id& value) const {
	std::size_t n_vars = _index.size();
	assert(variable < n_vars);

	// If the state is fully boolean or fully multivalued, we can optimize the operation,
	// since the variable index will be exactly `variable`
	if (n_vars == _n_bool) state._bool_values[variable] = bool(value);
	else if (n_vars == _n_int) state._int_values[variable] = value;
	else {
		const IndexElemT& ind = _index[variable];
		if (ind.first) state._bool_values[ind.second] = bool(value);
		else state._int_values[ind.second] = value;
	}
}

State* State::create(const StateAtomIndexer& index, unsigned numAtoms, const std::vector<Atom>& atoms) {
	assert(numAtoms == index.size());
	return new State(index, atoms);
}

State::State(const StateAtomIndexer& index, const std::vector<Atom>& atoms) :
	_indexer(index),
	_bool_values(index.num_bool(), 0),
	_int_values(index.num_int(), object_id::INVALID)
{
	// Note that those facts not explicitly set in the initial state will be initialized to 0, i.e. "false", which is convenient to us.
	for (const Atom& atom:atoms) { // Insert all the elements of the vector
		set(atom);
	}
	updateHash();
}

State::State(const State& state, const std::vector<Atom>& atoms) :
	State(state) {
	accumulate(atoms);
}

void State::set(const Atom& atom) {
// 	_bool_values.at(atom.getVariable()) = value;
#ifdef DEBUG
	const ProblemInfo& info = ProblemInfo::getInstance();
	assert( info.sv_type(atom.getVariable()) == o_type(atom.getValue()) );
#endif
	_indexer.set(*this, atom);
}

bool State::contains(const Atom& atom) const {
	return getValue(atom.getVariable()) == atom.getValue();
}

object_id
State::getValue(const VariableIdx& variable) const {
	return _indexer.get(*this, variable);
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
    for (unsigned x = 0; x < info.getNumVariables(); ++x) {
        object_id o = getValue(x);

        if (o_type(o) == type_id::bool_t) {
			if (value<bool>(o)) {
				os << info.getVariableName(x); // print positive atoms only
				if (x < info.getNumVariables() - 1) os << ", ";
			}
        } else {
			os << info.getVariableName(x) << "=" << info.object_name(o);
			if (x < info.getNumVariables() - 1) os << ", ";
		}
    }
	os << "]";
	return os;
}


std::size_t State::computeHash() const {
	//return std::hash<BitsetT>{}(_bool_values);
//     auto a = boost::hash_value( _bool_values);
//     auto b = boost::hash_value( _int_values);
// 	return boost::hash_value(_bool_values);
	std::size_t seed = 0;
	boost::hash_combine(seed, std::hash<BitsetT>{}(_bool_values));
	boost::hash_combine(seed, boost::hash_value( _int_values));
	return seed;

}


template <>
const std::vector<bool>&
State::dump() const {
	assert(_indexer.is_fully_binary());
	return _bool_values;
}

template <>
const std::vector<object_id>&
State::dump() const {
	assert(_indexer.is_fully_multivalued());
	return _int_values;
}


} // namespaces
