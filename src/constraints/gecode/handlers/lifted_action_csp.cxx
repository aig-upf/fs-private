
#include <languages/fstrips/terms.hxx>
#include <constraints/gecode/handlers/lifted_action_csp.hxx>
#include <actions/actions.hxx>
#include <aptk2/tools/logging.hxx>
#include <actions/action_id.hxx>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSP>>
LiftedActionCSP::create(const std::vector<const PartiallyGroundedAction*>& schemata, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	// Simply upcast the shared_ptrs
	std::vector<std::shared_ptr<BaseActionCSP>> handlers;
	for (const auto& element:create_derived(schemata, tuple_index, approximate, novelty)) {
		handlers.push_back(std::static_pointer_cast<BaseActionCSP>(element));
	}
	return handlers;
}

std::vector<std::shared_ptr<LiftedActionCSP>>
LiftedActionCSP::create_derived(const std::vector<const PartiallyGroundedAction*>& schemata, const TupleIndex& tuple_index, bool approximate, bool novelty) {
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


LiftedActionCSP::LiftedActionCSP(const PartiallyGroundedAction& action, const TupleIndex& tuple_index, bool approximate, bool use_effect_conditions)
:  BaseActionCSP(tuple_index, approximate, use_effect_conditions), _action(action)
{}


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
	for (unsigned i = 0; i < signature.size(); ++i) {
		if (_action.isBound(i)) {
			// If the parameter is bound, we mark it specially so that we know the value cannot be retrieved from the CSP,
			// because there will be no CSP variable modelling that parameter
			_parameter_variables.push_back(std::numeric_limits<unsigned int>::max());
		} else {
			// We here assume that the parameter IDs are always 0..k-1, where k is the number of parameters
			fs::BoundVariable variable(i, signature[i]);
			_parameter_variables.push_back(_translator.resolveVariableIndex(&variable));
		}
	}
}

Binding LiftedActionCSP::build_binding_from_solution(const GecodeCSP* solution) const {
	std::vector<int> values;
	std::vector<bool> valid;
	values.reserve(_parameter_variables.size());
	valid.reserve(_parameter_variables.size());
	for (unsigned csp_var_idx:_parameter_variables) {
		if (csp_var_idx == std::numeric_limits<unsigned int>::max()) {
			values.push_back(0);
			valid.push_back(false);
		} else {
			values.push_back(_translator.resolveValueFromIndex(csp_var_idx, *solution));
			valid.push_back(true);
		}
	}
	return Binding(std::move(values), std::move(valid));
}

const std::vector<const fs::ActionEffect*>& LiftedActionCSP::get_effects() const {
	return _action.getEffects();
}

const fs::Formula* LiftedActionCSP::get_precondition() const {
	return _action.getPrecondition();
}

// Simply forward to the more concrete method
const ActionID* LiftedActionCSP::get_action_id(const GecodeCSP* solution) const {
	return get_lifted_action_id(solution);
}

LiftedActionID* LiftedActionCSP::get_lifted_action_id(const GecodeCSP* solution) const {
	return new LiftedActionID(&_action, build_binding_from_solution(solution));
}

void LiftedActionCSP::log() const {
	LPT_EDEBUG("heuristic", "Processing action schema " << _action);
}

} } // namespaces
