
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/constraints/gecode/handlers/lifted_action_csp.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/actions/action_id.hxx>
#include <utility>

namespace fs0::gecode {

std::vector<std::shared_ptr<LiftedActionCSP>>
LiftedActionCSP::create(const std::vector<const PartiallyGroundedAction*>& schemata, const AtomIndex& tuple_index, bool approximate, bool novelty) {
    std::vector<std::shared_ptr<LiftedActionCSP>> handlers;

    for (auto schema:schemata) {
        assert(!schema->has_empty_parameter());
        // When creating an action CSP handler, it doesn't really make much sense to use the effect conditions.
        auto handler = std::make_shared<LiftedActionCSP>(*schema, tuple_index, approximate, false);

        if (!handler->init(novelty)) {
            LPT_DEBUG("grounding", "Action schema \"" << *schema << "\" detected as non-applicable before grounding");
            continue;
        }

        LPT_DEBUG("grounding", "Generated CSP for action schema " << *schema << std::endl <<  *handler << std::endl);
        handlers.push_back(handler);
    }
    return handlers;
}


LiftedActionCSP::LiftedActionCSP(const PartiallyGroundedAction& action, const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions)
    : LiftedActionCSP(action, extract_non_delete_effects(action), tuple_index, approximate, use_effect_conditions)
{}

LiftedActionCSP::LiftedActionCSP(const PartiallyGroundedAction& action, std::vector<const fs::ActionEffect*>  effects, const AtomIndex& tuple_index, bool approximate, bool use_effect_conditions)
    :  BaseActionCSP(tuple_index, approximate, use_effect_conditions), _action(action), _effects(std::move(effects))
{}

LiftedActionCSP::~LiftedActionCSP() {
    for (auto eff:_effects) delete eff;
}


std::vector<const fs::ActionEffect*> LiftedActionCSP::extract_non_delete_effects(const PartiallyGroundedAction& action) {
    std::vector<const fs::ActionEffect*> effects;
    for (const fs::ActionEffect* effect:action.getEffects()) {
        if (!effect->is_del()) {
            effects.push_back(effect->clone());
        }
    }
    return effects;
}


bool LiftedActionCSP::init(bool use_novelty_constraint) {
    if (BaseActionCSP::init(use_novelty_constraint)) {
        index_parameters();
        return true;
    }
    return false;
}


void LiftedActionCSP::index_parameters() {
    // Index in '_parameter_variables' the (ordered) CSP variables that correspond to the action parameters
    const Signature& signature = _action.getSignature();
    const fs::BindingUnit& unit =_action.getBindingUnit();
    for (unsigned i = 0; i < signature.size(); ++i) {
        if (_action.isBound(i)) {
            // If the parameter is bound, we mark it specially so that we know the value cannot be retrieved from the CSP,
            // because there will be no CSP variable modelling that parameter
            _parameter_variables.push_back(std::numeric_limits<unsigned int>::max());
        } else {
            // We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
            _parameter_variables.push_back(_translator.resolveVariableIndex(unit.getParameterVariable(i)));
        }
    }
}

void
LiftedActionCSP::register_csp_variables() {
    BaseCSP::register_csp_variables();
    // We simply make sure all action parameters have been registered.
    const Signature& signature = _action.getSignature();
    const fs::BindingUnit& unit =_action.getBindingUnit();
    for (unsigned i = 0; i < signature.size(); ++i) {
        if (!_action.isBound(i)) {
            // We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
            const fs::BoundVariable* variable = unit.getParameterVariable(i);
            if (!_translator.isRegistered(variable)) {
                registerTermVariables(variable,  _translator);
            }
        }
    }
}



Binding LiftedActionCSP::build_binding_from_solution(const FSGecodeSpace* solution) const {
    std::vector<object_id> values;
    std::vector<bool> valid;
    values.reserve(_parameter_variables.size());
    valid.reserve(_parameter_variables.size());
    for (unsigned csp_var_idx:_parameter_variables) {
        if (csp_var_idx == std::numeric_limits<unsigned int>::max()) {
            values.push_back(object_id::INVALID);
            valid.push_back(false);
        } else {
            values.push_back(_translator.resolveValueFromIndex(csp_var_idx, *solution));
            valid.push_back(true);
        }
    }
    return Binding(std::move(values), std::move(valid));
}

const std::vector<const fs::ActionEffect*>& LiftedActionCSP::get_effects() const {
    return _effects;
}

const fs::Formula* LiftedActionCSP::get_precondition() const {
    return _action.getPrecondition();
}

// Simply forward to the more concrete method
const ActionID* LiftedActionCSP::get_action_id(const FSGecodeSpace* solution) const {
    return get_lifted_action_id(solution);
}

LiftedActionID* LiftedActionCSP::get_lifted_action_id(const FSGecodeSpace* solution) const {
    return new LiftedActionID(&_action, build_binding_from_solution(solution));
}

void LiftedActionCSP::log() const {
    LPT_EDEBUG("heuristic", "Processing action schema " << _action);
}

} // namespaces
