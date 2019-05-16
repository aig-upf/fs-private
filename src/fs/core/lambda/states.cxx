
#include <fs/core/lambda/states.hxx>

#include <boost/functional/hash.hpp>
//#include <fs/core/problem_info.hxx>
//#include <fs/core/atom.hxx>

//#include <utility>

namespace lambda { namespace states {


BinaryState::BinaryState(const bitset_t& data) :
        data_(data), hash_(computeHash())
{}

BinaryState::BinaryState(bitset_t&& data) :
        data_(std::move(data)), hash_(computeHash())
{}

/*
void BinaryState::set(const Atom& atom) {
 	data_.at(atom.getVariable()) = value;
#ifdef DEBUG
    const ProblemInfo& info = ProblemInfo::getInstance();
assert( info.sv_type(atom.getVariable()) == o_type(atom.getValue()) );
#endif
}

bool BinaryState::contains(const Atom& atom) const {
    return getValue(atom.getVariable()) == atom.getValue();
}

object_id
BinaryState::getValue(const VariableIdx& variable) const {
    return _indexer.get(*this, variable);
}
*/

std::ostream& BinaryState::print(std::ostream& os) const {
    os << "State [UNIMPLEMENTED]";
    /*
    const ProblemInfo& info = ProblemInfo::getInstance();
    os << "(" << hash_ << ")[";
    for (unsigned x = 0; x < info.getNumVariables(); ++x) {
        object_id o = getValue(x);

        if (o_type(o) == type_id::bool_t) {
            if (value<bool>(o)) {
                os << info.getVariableName(x); // print positive atoms only
                if (x < info.getNumVariables() - 1) os << ", ";
            }
        } else {
            os << info.getVariableName(x) << "=";
            if (o_type(o) == type_id::invalid_t)
                os << "<invalid>";
            else
                os << info.object_name(o);
            if (x < info.getNumVariables() - 1) os << ", ";
        }
    }
    os << "]";
    */
    return os;
}


std::size_t BinaryState::computeHash() const {
    //return std::hash<bitset_t>{}(data_);
//     auto a = boost::hash_value( data_);
//     auto b = boost::hash_value( _int_values);
// 	return boost::hash_value(data_);
    std::size_t seed = 0;
    boost::hash_combine(seed, std::hash<bitset_t>{}(data_));
    return seed;

}


template <>
const std::vector<bool>&
BinaryState::dump() const {
    return data_;
}

} } // namespaces
