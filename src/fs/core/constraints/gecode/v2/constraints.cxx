
#include <fs/core/constraints/gecode/v2/constraints.hxx>
#include <fs/core/constraints/gecode/v2/gecode_space.hxx>
#include <fs/core/state.hxx>

namespace fs0::gecode::v2 {


RelationalConstraint::RelationalConstraint(int lhs, Gecode::IntRelType reltype, int rhs) :
    lhs(lhs), rhs(rhs), reltype(reltype)
{
}


bool TableConstraint::post(FSGecodeSpace& csp, const Gecode::TupleSet& extension) const {
    assert(extension.finalized());

    // If the extension of the constraint is empty and it is not a negative constraint, flag the CSP as unsolvable
    if (!negative && extension.tuples() == 0) return false;

    Gecode::IntVarArgs variables;
    for (auto i:varidxs) {
        variables << csp.intvars[i];
    }

    // Post the constraint with the given extension
    Gecode::extensional(csp, variables, extension, !negative);
    return true;
}



bool StateVariableConstraint::post(const State& state) const {
    return state.getValue(varidx) == value;
}


bool RelationalConstraint::post(FSGecodeSpace& csp) const {
    Gecode::rel(csp, csp.intvars[lhs], reltype, csp.intvars[rhs]);
    return true;
}


std::ostream& TableConstraint::print(std::ostream& os) const {
    os << "Table Constraint" << std::endl;
    return os;
}

std::ostream& StateVariableConstraint::print(std::ostream& os) const {
    os << "StateVariableConstraint" << std::endl;
    return os;
}

std::ostream& RelationalConstraint::print(std::ostream& os) const {
    os << "RelationalConstraint" << std::endl;
    return os;
}

} // namespaces
