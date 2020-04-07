
#pragma once

#include <fs/core/base.hxx>

#include <gecode/int.hh>

#include <vector>
#include <ostream>


namespace Gecode { class TupleSet; }
namespace fs0 { class State; }

namespace fs0::gecode::v2 {

class FSGecodeSpace;

class CSPConstraint {
//    virtual bool requires_gecode() = 0;

    //! Post the constraint given the information on the state
//    virtual bool post(FSGecodeSpace& csp, const State& state) const = 0;

};


class TableConstraint : public CSPConstraint {
public:
	TableConstraint(unsigned symbolidx, std::vector<int> varidxs, bool negative) :
            symbolidx(symbolidx), varidxs(std::move(varidxs)), negative(negative) {}
	TableConstraint(const TableConstraint&) = default;
	TableConstraint(TableConstraint&&) = default;
	TableConstraint& operator=(const TableConstraint&) = default;
	TableConstraint& operator=(TableConstraint&&) = default;

	//! Constraint-posting routines
	bool post(FSGecodeSpace& csp, const Gecode::TupleSet& extension) const;

	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const TableConstraint&  o) { return o.print(os); }
	std::ostream& print(std::ostream& os) const;

	unsigned symbol_idx() const { return symbolidx; }
	
protected:
    //!
    unsigned symbolidx;

    //! The (integer) CSP variables involved in the constraint
    std::vector<int> varidxs;

	//! Whether the extensional constraint should be posted in negative form
	bool negative;
};

class StateVariableConstraint {
public:
    StateVariableConstraint(unsigned varidx, object_id value) : varidx(varidx), value(value) {}
    StateVariableConstraint(const StateVariableConstraint&) = default;
    StateVariableConstraint(StateVariableConstraint&&) = default;
    StateVariableConstraint& operator=(const StateVariableConstraint&) = default;
    StateVariableConstraint& operator=(StateVariableConstraint&&) = default;

    //! Constraint-posting routines
    bool post(const State& state) const;

    //! Prints a representation of the state to the given stream.
    friend std::ostream& operator<<(std::ostream &os, const StateVariableConstraint&  o) { return o.print(os); }
    std::ostream& print(std::ostream& os) const;

    unsigned varidx;
    object_id value;
};

class RelationalConstraint {
public:
    RelationalConstraint(int lhs, Gecode::IntRelType reltype, int rhs);
    RelationalConstraint(const RelationalConstraint&) = default;
    RelationalConstraint(RelationalConstraint&&) = default;
    RelationalConstraint& operator=(const RelationalConstraint&) = default;
    RelationalConstraint& operator=(RelationalConstraint&&) = default;

    //! Constraint-posting routines
    bool post(FSGecodeSpace& csp) const;

    //! Prints a representation of the state to the given stream.
    friend std::ostream& operator<<(std::ostream &os, const RelationalConstraint&  o) { return o.print(os); }
    std::ostream& print(std::ostream& os) const;

    int lhs;
    int rhs;
    Gecode::IntRelType reltype;
};


} // namespaces
