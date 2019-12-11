

#include <fs/core/actions/propositional_actions.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/atom.hxx>



namespace fs0 {

const SchematicActionID SchematicActionID::invalid_action_id = SchematicActionID(nullptr, std::vector<object_id>());

SimpleGroundOperator SchematicActionID::ground() const {
    return action_->ground(binding_);
}


void
compute_effects(const SimpleGroundOperator& op, std::vector<Atom>& effects) {
    // Simply accumulate first the atoms that derive from add effects, then those of del effects
    const auto& add_effects = op.add_effects();
    const auto& del_effects = op.del_effects();
    effects.clear();
    effects.reserve(add_effects.size()+del_effects.size());

    for (const VariableIdx& var:add_effects) {
        effects.emplace_back(var, object_id::TRUE);
#ifdef DEBUG
        ProblemInfo::getInstance().checkValueIsValid(effects.back().getVariable(), effects.back().getValue());
#endif
    }

    for (const VariableIdx& var:del_effects) {
        effects.emplace_back(var, object_id::FALSE);
#ifdef DEBUG
        ProblemInfo::getInstance().checkValueIsValid(effects.back().getVariable(), effects.back().getValue());
#endif
    }
}

std::pair<VariableIdx, object_id> SimpleSchematicEffect::ground() const {

//    return std::pair<VariableIdx, object_id>();
}
} // namespaces
