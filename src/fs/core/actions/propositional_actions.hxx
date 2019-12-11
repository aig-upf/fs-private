
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/binding.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/applicability/action_managers.hxx>
#include <fs/core/languages/fstrips/axioms.hxx>
#include <utility>

#include <boost/functional/hash.hpp>


namespace fs0 {

class ProblemInfo;
class Atom;
class State;

class GroundOperator {
public:
    GroundOperator() {}

    const ActionData& base() const {
        return *base_;
    }

protected:
    std::shared_ptr<ActionData> base_;

    unsigned delete_effects_until_;
    std::vector<VariableIdx> effects_;
};

class SimpleGroundOperator : public GroundOperator {
public:
    SimpleGroundOperator(std::vector<VariableIdx> add_effects, std::vector<VariableIdx> delete_effects)
        : add_effects_(std::move(add_effects)), del_effects_(std::move(delete_effects))
    {}


    const std::vector<VariableIdx>& add_effects() const { return add_effects_; }
    const std::vector<VariableIdx>& del_effects() const { return del_effects_; }

protected:
    std::vector<VariableIdx> add_effects_;
    std::vector<VariableIdx> del_effects_;
};

void compute_effects(const SimpleGroundOperator& op, std::vector<Atom>& effects);

class SchematicOperator {
public:
    explicit SchematicOperator(const ActionData* data) {}


    const ActionData& data() const {
        return *data_;
    }

protected:
    const ActionData* data_;
};

class SimpleSchematicEffect {
public:
    SimpleSchematicEffect() = default;

    const std::vector<VariableIdx>& inputs() const { return inputs_; }

    std::pair<VariableIdx, object_id> ground(std::vector<object_id>& values) const;

protected:
    std::vector<VariableIdx> inputs_;

    using key_t = std::vector<object_id>;
    std::unordered_map<key_t, std::pair<VariableIdx, object_id>, boost::hash<key_t>> mapping_;
};

class SimpleSchematicOperator : public SchematicOperator {
public:
    SimpleSchematicOperator(const ActionData* data) :
            SchematicOperator(data)
    {}


    SimpleGroundOperator ground(const Binding& binding) const;
protected:
    std::vector<SimpleSchematicEffect> effects;
};


//! An action is fully identified by the ID of the action schema and the values of its parameters,
//! i.e. its binding
class SchematicActionID {
protected:
    //!
    const SimpleSchematicOperator* action_;

    //! The indexes of the action binding.
    Binding binding_; // This could be const, but then we cannot have assignment operator

public:
    using IdType = SchematicActionID;
    static const SchematicActionID invalid_action_id;

    SchematicActionID(const SimpleSchematicOperator* action, Binding&& binding) :
        action_(action), binding_(binding)
    {}

    ~SchematicActionID() = default;
    SchematicActionID(const SchematicActionID& other) = default;
    SchematicActionID(SchematicActionID&& other) = default;
    SchematicActionID& operator=(const SchematicActionID& other) = default;
    SchematicActionID& operator=(SchematicActionID&& other) = default;

//    bool operator==(const SchematicOperator& rhs) const;

    //! Prints a representation of the object to the given stream.
//    std::ostream& print(std::ostream& os) const;

    const Binding& binding() const {
        return binding_;
    }

    const ActionData& data() const {
        return action_->data();
    }

    SimpleGroundOperator ground() const;
};


} // namespaces
