
#pragma once

#include <fs/core/constraints/gecode/handlers/base_action_csp.hxx>
#include <fs/core/actions/actions.hxx> // Necessary so that the return of get_action can be identified as covariant with that of the overriden method

namespace fs0 { class LiftedActionID; class PartiallyGroundedAction; }

namespace fs0::language::fstrips { class ActionEffect; }
namespace fs = fs0::language::fstrips;

namespace fs0::gecode {


//! A CSP modeling and solving the effect of an action on a certain RPG layer
class LiftedActionCSP : public BaseActionCSP {
public:
    //! Factory method
    static std::vector<std::shared_ptr<BaseActionCSP>> create(const std::vector<const PartiallyGroundedAction*>& schemata, const AtomIndex& tuple_index, bool approximate, bool novelty);

    //! HACK
    static std::vector<std::shared_ptr<LiftedActionCSP>> create_derived(const std::vector<const PartiallyGroundedAction*>& schemata, const AtomIndex& tuple_index, bool approximate, bool novelty);

    LiftedActionCSP(const PartiallyGroundedAction& action, const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions);
    LiftedActionCSP(const PartiallyGroundedAction& action, std::vector<const fs::ActionEffect*>  effects, const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions);
    ~LiftedActionCSP() override;
    LiftedActionCSP(const LiftedActionCSP&) = delete;
    LiftedActionCSP(LiftedActionCSP&&) = delete;
    LiftedActionCSP& operator=(const LiftedActionCSP&) = delete;
    LiftedActionCSP& operator=(LiftedActionCSP&&) = delete;

    bool init(bool use_novelty_constraint) override;

    const PartiallyGroundedAction& get_action() const override { return _action; }

    const std::vector<const fs::ActionEffect*>& get_effects() const override;

    const fs::Formula* get_precondition() const override;

    //! Return the (Lifted) ActionID corresponding to the given solution
    LiftedActionID* get_lifted_action_id(const GecodeCSP* solution) const;

protected:
    //! We override the base method to ensure all action parameters are registered as CSP variables
    //! even if they do not appear in other parts of the CSP
    void register_csp_variables() override;

    //! The action that originates this handler
    const PartiallyGroundedAction _action;

    //! The effects of the action which are relevant to the CSP manaer (typically these will exclude predicative delete effects!)
    const std::vector<const fs::ActionEffect*> _effects;

    //! '_parameter_variables[i]' contains the index of the CSP variable that models the value of i-th parameter of the action schema
    std::vector<unsigned> _parameter_variables;


    static std::vector<const fs::ActionEffect*> extract_non_delete_effects(const PartiallyGroundedAction& action);

    //! An schema handler needs to index the action parameter CSP variables in addition
    //! to the other elements already indexed by the parent class
    void index_parameters();

    //! Return the action binding that corresponds to the given solution
    Binding build_binding_from_solution(const GecodeCSP* solution) const override;

    //! Return the (Lifted) ActionID corresponding to the given solution
    const ActionID* get_action_id(const GecodeCSP* solution) const override;

    //! Log some handler-related into
    void log() const override;
};

} // namespaces
