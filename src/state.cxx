
#include <boost/functional/hash.hpp>

#include <state.hxx>
#include <problem_info.hxx>
#include <atom.hxx>
#include <lapkt/tools/logging.hxx>
#include <utils/utils.hxx>


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

	return new StateAtomIndexer(std::move(index), n_bool, n_int, info);
}

StateAtomIndexer::StateAtomIndexer(IndexT&& index, unsigned n_bool, unsigned n_int, const ProblemInfo& info) :
	_index(std::move(index)), _n_bool(n_bool), _n_int(n_int), _info(info)
{
}

ObjectIdx
StateAtomIndexer::get(const State& state, VariableIdx variable) const {
	std::size_t n_vars = _index.size();
	assert(variable < n_vars);

	// If the state is fully boolean or fully multivalued, we can optimize the operation,
	// since the variable index will be exactly `variable`
	if (n_vars == _n_bool) return state._bool_values[variable];
	if (n_vars == _n_int) {
        if ( _info.isIntegerNumber(variable) )
            return ObjectIdx(state._int_values[variable]);
        else if (_info.isRationalNumber(variable) ) {
            float tmp;
            Utils::type_punning_without_aliasing( state._int_values[variable], tmp );
            return ObjectIdx(tmp);
        }
        else {
            std::stringstream buffer;
            buffer << "ERROR: Cannot retrieve variable from state ";
            buffer << _info.getVariableName(variable) << std::endl;
            buffer << "because its type is not currently supported!" << std::endl;
            LPT_DEBUG("main", buffer.str());
            throw std::runtime_error(buffer.str());
        }
        return state._int_values[variable];
    }

	// Otherwise we need to deindex the variable
	const IndexElemT& ind = _index[variable];
	if (ind.first)
        // MRJ: Booleans get expanded to integers
        return ObjectIdx((int)state._bool_values[ind.second]);
	else {
        if ( _info.isIntegerNumber(variable) )
            return ObjectIdx(state._int_values[ind.second]);
        else if (_info.isRationalNumber(variable) ) {
            float tmp;
            Utils::type_punning_without_aliasing( state._int_values[ind.second], tmp );
            return ObjectIdx(tmp);
        }
        else {
            std::stringstream buffer;
            buffer << "ERROR: Cannot retrieve variable from state ";
            buffer << _info.getVariableName(variable) << std::endl;
            buffer << "because its type is not currently supported!" << std::endl;
            LPT_DEBUG("main", buffer.str());
            throw std::runtime_error(buffer.str());
        }
    }
    return ObjectIdx(0);
}

void
StateAtomIndexer::set(State& state, const Atom& atom) const {
    ObjectIdx tmp =  atom.getValue();
    set(state,  atom.getVariable(), boost::apply_visitor( Utils::reinterpreted_as_int(), tmp));
}

void
StateAtomIndexer::set(State& state, VariableIdx variable, ObjectIdx value) const {
	std::size_t n_vars = _index.size();
	assert(variable < n_vars);

	// If the state is fully boolean or fully multivalued, we can optimize the operation,
	// since the variable index will be exactly `variable`
	if (n_vars == _n_bool) state._bool_values[variable] = (bool)boost::get<int>(value);
	else if (n_vars == _n_int) state._int_values[variable] = boost::get<int>(value);
	else {
		const IndexElemT& ind = _index[variable];
		if (ind.first)
            state._bool_values[ind.second] = (bool)boost::get<int>(value);
		else {
            if ( _info.isIntegerNumber(variable) )
                state._int_values[ind.second] = boost::get<int>(value);
            else if (_info.isRationalNumber(variable)) {
                union {
    				int i;
    				float d;
    			} tmp;
                tmp.d = boost::get<float>(value);
                state._int_values[ind.second] = tmp.i;
            }
            else {
                std::stringstream buffer;
                buffer << "ERROR: Cannot store variable into state ";
                buffer << _info.getVariableName(variable) << std::endl;
                buffer << "because its type is not currently supported!" << std::endl;
                LPT_DEBUG("main", buffer.str());
                throw std::runtime_error(buffer.str());
            }
        }
	}
}

State* State::create(const StateAtomIndexer& index, unsigned numAtoms, const std::vector<Atom>& atoms) {
	assert(numAtoms == index.size());
	return new State(index, atoms);
}

State::State(const StateAtomIndexer& index, const std::vector<Atom>& atoms) :
	_indexer(&index),
	_bool_values(index.num_bool(), 0),
	_int_values(index.num_int(), 0)
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
	_indexer->set(*this, atom);
}

void State::set(VariableIdx x, ObjectIdx v) {
    set(Atom(x,v));
}

bool State::contains(const Atom& atom) const {
	return boost::get<int>(getValue(atom.getVariable())) == boost::get<int>(atom.getValue());
}

ObjectIdx
State::getValue(const VariableIdx& variable) const {
	return _indexer->get(*this, variable);
}

int
State::getIntValue(const VariableIdx& variable) const {
    ObjectIdx tmp = _indexer->get(*this, variable);
    return boost::apply_visitor( Utils::reinterpreted_as_int(), tmp );
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
    for ( unsigned x = 0; x < info.getNumVariables(); x++ ) {
        ObjectIdx v = getValue(x);
        if ( info.getVariableGenericType(x) == ProblemInfo::ObjectType::BOOL ) {
            if ( boost::get<int>(v) == 0 ) continue;
            os << info.getVariableName(x);
    		if (x < info.getNumVariables() - 1) os << ", ";
            continue;
        }
        else if ( info.getVariableGenericType(x) == ProblemInfo::ObjectType::INT ) {
            os << info.getVariableName(x) << "=" << v;
            if (x < info.getNumVariables() - 1) os << ", ";
            continue;
        }
        os << info.getVariableName(x) << "=" << info.getObjectName(x, v);
        if (x < info.getNumVariables() - 1) os << ", ";
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


} // namespaces
