
#pragma once

#include "constraints.hxx"

#include <memory>

namespace fs0 {
    class LiftedActionID;
    class PartiallyGroundedAction;
    class ProblemInfo;
}

namespace fs0::gecode::v2 {

class FSGecodeSpace;
class SymbolExtensionGenerator;

//! A CSP modeling and solving the effect of an action on a certain RPG layer
class ActionSchemaCSP {
protected:
    ActionSchemaCSP();

public:
    ~ActionSchemaCSP() = default;
    ActionSchemaCSP(const ActionSchemaCSP& other) = default;
    ActionSchemaCSP(ActionSchemaCSP&&) = default;
    ActionSchemaCSP& operator=(const ActionSchemaCSP&) = default;
    ActionSchemaCSP& operator=(ActionSchemaCSP&&) = default;

    //! Load an object from a serialized representation
    static ActionSchemaCSP load(std::ifstream& in, const ProblemInfo& info, std::vector<unsigned>& symbols_in_extensions);

    //! Post all those constraints that do not depend on any particular state.
    //! Return true iff the underlying CSP is locally consistent after that propagation.
    //! This method needs to be called before doing any other operation on the CSP, as performs the mandatory constraint
    //! propagation of the underlying Gecode CSP.
    bool initialize();

    //! Clone the underlying CSP and post *on the clone* those constraints that depend on the given state and
    //! set of extensions (which in turn depend on the state, but it's good for performance reasons to compute them
    //! once for all interested CSPs). Return the cloned CSP, or null if it is not locally consistent and hence we
    //! know can't have any solution.
    FSGecodeSpace* instantiate(const State& state, const SymbolExtensionGenerator& extension_generator) const;

    //! Return the action binding that corresponds to the given solution
    std::vector<object_id> build_binding_from_solution(const FSGecodeSpace* solution) const;

protected:
    //! The base Gecode CSP
    std::shared_ptr<FSGecodeSpace> space;

    //! 'parameter_variables_[i]' contains the index of the integer CSP variable that models the value of i-th parameter of the action schema
    std::vector<int> parameter_variables;

    //! We store the different types of constraints that we support separately
    std::vector<TableConstraint> table_constraints;
    std::vector<StateVariableConstraint> statevar_constraints;
    std::vector<RelationalConstraint> relational_constraints;
};

} // namespaces
