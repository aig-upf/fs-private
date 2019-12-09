
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/binding.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/applicability/action_managers.hxx>
#include <fs/core/languages/fstrips/axioms.hxx>


namespace fs0 {

class ProblemInfo;
class Atom;
class State;

class PropositionalGroundAction {
public:
    PropositionalGroundAction() {}

    const ActionData& base() const {
        return *base_;
    }

protected:
    std::shared_ptr<ActionData> base_;

    unsigned delete_effects_until_;
    std::vector<VariableIdx> effects_;
};

class PropositionalSchematicAction {
public:
    explicit PropositionalSchematicAction(const ActionData* data) {}


    const ActionData& data() const {
        return *data_;
    }

protected:
    const ActionData* data_{};
};


/*
class SimpleGroundAction {
public:
    using effect_t = std::pair<VariableIdx, object_id>;
    SimpleGroundAction() {}


protected:
    std::vector<effect_t> effects_;
};
*/


//! An action is fully identified by the ID of the action schema and the values of its parameters,
//! i.e. its binding
class SchematicActionID {
protected:
    //!
    const PropositionalSchematicAction* action_;

    //! The indexes of the action binding.
    Binding binding_; // This could be const, but then we cannot have assignment operator

public:
    using IdType = SchematicActionID;
    static const SchematicActionID invalid_action_id;

    SchematicActionID(const PropositionalSchematicAction* action, Binding&& binding) :
        action_(action), binding_(binding)
    {}

    ~SchematicActionID() = default;

    //! Default copy constructors and assignment operators
    SchematicActionID(const SchematicActionID& other) = default;
    SchematicActionID(SchematicActionID&& other) = default;
    SchematicActionID& operator=(const SchematicActionID& other) = default;
    SchematicActionID& operator=(SchematicActionID&& other) = default;

//    bool operator==(const PropositionalSchematicAction& rhs) const;

    //! Prints a representation of the object to the given stream.
//    std::ostream& print(std::ostream& os) const;

    const Binding& get_binding() const {
        return binding_;
    }

    const ActionData& data() const {
        return action_->data();
    }

//    generate_effect() const;
};


} // namespaces
