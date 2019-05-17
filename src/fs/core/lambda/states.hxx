
#pragma once

//#include <fs/core/fs_types.hxx>
// #include <fs/core/utils/bitsets.hxx>

#include <vector>
#include <ostream>

namespace lambda::search {

class State {
public:
    //! Return the number of variables in the state
    virtual std::size_t num_atoms() const = 0;

    //! Return the hash value of the state
    virtual std::size_t hash() const = 0;

    //! Print a representation of the state to the given stream.
    friend std::ostream& operator<<(std::ostream &os, const State& o) { return o.print(os); }
    virtual std::ostream& print(std::ostream& os) const = 0;
};


class BinaryState : public State {
public:
    // using bitset_t = boost::dynamic_bitset<>;
    using bitset_t = std::vector<bool>;

    //! Construct a state specifying the values of all state variables
    //! Note that it is not necessarily the case that numAtoms == atoms.size(); since the initial values of
    //! some (Boolean) state variables is often left unspecified and understood to be false.
    BinaryState(const bitset_t& data);
    BinaryState(bitset_t&& data);
    ~BinaryState() = default;

    //! Default copy constructors and assignment operators
    BinaryState(const BinaryState&) = default;
    BinaryState(BinaryState&&) = default;
    BinaryState& operator=(const BinaryState&) = default;
    BinaryState& operator=(BinaryState&&) = default;

    // Check the hash first for performance.
    bool operator==(const BinaryState &rhs) const { return hash_ == rhs.hash_ && data_ == rhs.data_; }
    bool operator!=(const BinaryState &rhs) const { return !(this->operator==(rhs));}


//    bool contains(const Atom& atom) const;
//    object_id getValue(const VariableIdx& variable) const;


    std::size_t num_atoms() const override { return data_.size(); }

    template <typename ValueT>
    const std::vector<ValueT>& dump() const;

    const bitset_t& data() const { return data_; };

    std::size_t hash() const override { return hash_; }

    std::ostream& print(std::ostream& os) const override;

protected:
//    void set(const Atom& atom);

    std::size_t computeHash() const;

    //! A vector mapping state variable (implicit) ids to their value in the current state.
    bitset_t data_;

    std::size_t hash_;
};


} // namespaces
